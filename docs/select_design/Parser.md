### Parser

在此之前已经注意到关于字符串，以及其他类型的归约失败容易导致的内存泄漏；

但在生成的c源文件中使用智能指针是困难的， 因此此处在`ParserContext` 加入一个字符串栈，`lexer`读入则入栈，`bison`归约时，一定是从栈顶出栈。

默认生成的是`LALR(1)` 文法，从左到右识别，因此只要把每个`ID`与`STRING`都吃入即可保证正确性。

相较于在每个`ID`等token值归约时手动释放，这种方式实现的复杂程度相当，但在内存方面显然更安全。

#### Flex

```flex
WHITE_SPACE		 [\ \t\b\r\f]
DIGIT            [0-9]

ID               [A-Za-z_]+[A-Za-z0-9_]*
DOT              \.
SINGLE_QUOTE     \'

%%
{WHITE_SPACE}     ; /* ignore whitespace*/
\n                ; 

select 			return SELECT;
from 			return FROM;
";"				return SEMICOLON;
","				return COMMA;
"*"				return STAR;
{ID}			return ID;
"."				return DOT;
where			return WHERE;
"("				return LBRACE;
")"				return RBRACE;
not				return NOT;
and				return AND;
or				return OR;
"="             return EQ;
"<="            return LE;
"<>"			return NE;
"<"				return LT;
">="			return GE;
">"				return GT;
[\-]?{DIGIT}+                  	yylval->number = atoi(yytext); return NUMBER;
[\-]?{DIGIT}+{DOT}{DIGIT}+ 		yylval->floats = (float)(atof(yytext)); return FLOAT;
{SINGLE_QUOTE}.*{SINGLE_QUOTE}	yylval->string = strdup(yytext); return STRING;
%%
```

#### Bison 

```yacc
%token  SELECT
		FROM
		SEMICOLON
		COMMA
		STAR 
		DOT
		WHERE
		LBRACE
		RBRACE
		NOT 
		AND
		OR
		EQ
		NE
		LE
		LT
		GE
		GT

%union {  
  char* string;
  int number;
  float floats;
}

%token <string> ID
%token <number> NUMBER
%token <floats> FLOAT 
%token <string> STRING

%left OR
%left AND
%right NOT
```

```yacc
/* production list */
command: 
	select;

select:
	SELECT sel_list FROM from_list where_clause SEMICOLON {}
	;

sel_list:
	sel_list_item {}
	|
	sel_list COMMA sel_list_item {}
	;

sel_list_item:
	STAR {}
	|
	ID DOT STAR {}
	| 
	attribute {}
	;

attribute:
	ID DOT ID {}
	|
	ID {}
	;
from_list:
	ID {}
	|
	from_list COMMA ID {}
	;

where_clause:
	/* empty */
	|
	WHERE conditions {}
	;

conditions:
	condition_operand compare_op compare_operand {}
	|
	LBRACE conditions RBRACE {}
	|
	NOT conditions {}
	|
	conditions AND conditions {}
	| 
	conditions OR conditions {}
 	;

compare_op: 
    EQ { $$ = CompareOp::OP_EQ; }
    | 
    LE { $$ = CompareOp::OP_LE; }
    |
    NE { $$ = CompareOp::OP_NE; }
    |
    LT { $$ = CompareOp::OP_LT; }
    | 
    GE { $$ = CompareOp::OP_GE; }
    |
    GT { $$ = CompareOp::OP_GT; }
    ;
    
condition_operand:
    attribute {}
    |
    value {}
    ;

value:
	NUMBER { 
        
    }
	|
	FLOAT {
         
    }
	|
	STRING {
        
    }
	;
```

#### Semantic

```cpp
enum class SqlType {
	Invalid,
    Select,
};

class Query {
public:
    auto get_sql_type() const -> SqlType = 0;
};

struct ParserContext {
  	std::unique_ptr<Query> query;
    std::list<std::string> str_queue_;
};

```

Select:

```cpp
class SelectQuery : public Query {
private:
    std::vector<Attribute> attributes_;
    std::vector<std::string> relations_;
	std::unique_ptr<Predicate> conditions_;
public:
    
};

class Attribute {
private:
    std::string relation_;
	std::string attribute_;
};
```

Conditions:

条件子句是一棵条件树。

```cpp
enum class PredicateType {
  CNT_NOT,
  CNT_AND,
  CNT_OR,
  CNT_LEAF,
};


class Predicate {
public:
    virtual auto get_node_type() const -> PredicateType = 0;
  	virtual ~Predicate() {}
};

class PredicateNot : public Predicate {
private:
  	std::unique_ptr<Predicate> child_; 	  
};

class PredicateAnd : public Predicate {
private:
  std::unique_ptr<Predicate> lchild_, rchild_;
};

class PredicateOr : public Predicate {
private:
  	std::unique_ptr<Predicate> lchild_, rchild_;    
};

enum class CompareOp {
  OP_EQ,
  OP_LE,
  OP_NE,
  OP_LT,
  OP_GE,
  OP_GT,
};

class PredicateLeaf : public Predicate {
private:
    CompareOp op_;
    std::unique_ptr<Operand> lchild_, rchild_;
}

enum class OperandType {
  COT_ATTR,
  COT_VAL,
};

class Operand {
public:
  virtual OperandType get_operand_type() const = 0;
  virtual ~Operand() {}
};

class OperandAttr : public Operand {
private:
  Attribute attr_;
};

class OperandVal : public Operand {
private:
  std::unique_ptr<Value> val_;
};

enum class ValueType {
  VT_STRING,
  VT_INT,
  VT_FLOAT,
  VT_DATE,
};

class Value {
public:
    auto get_value_type() const -> ValueType = 0;
};

```

