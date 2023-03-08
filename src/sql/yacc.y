%{

#include "sql/parser.h"
/* this must be prior to lexer.yy.h because the latter depends on this header*/
#include "sql/yacc.tab.h" 
#include "sql/lexer.yy.h"

void yyerror(yyscan_t scanner, const char *str) {
  printf("parse sql failed. error=%s", str);
}

ParserContext *get_context(yyscan_t scanner) {
  return (ParserContext *)yyget_extra(scanner);
}

#define CONTEXT get_context(scanner)
%}
/* */
%define api.pure full
%lex-param { yyscan_t scanner }
%parse-param { void *scanner }

%token  EXIT
        SEMICOLON
        HELP 
        SHOW
        TABLES
        DROP
        TABLE
        DESC
        INDEX
        CREATE
        ON
        LBRACE
        RBRACE
        COMMA        
        INT_T
        STRING_T
        FLOAT_T 
        DATE_T
        INSERT
        INTO
        VALUES
        UPDATE
        SET
        AND
        OR
        NOT
        EQ
        LE
        NE
        LT
        GE
        GT
        DOT
        DELETE
        FROM
        SELECT
        STAR 
        WHERE   
        
%left OR
%left AND
%right NOT
/* YYSTYPE */
%union {  
  char* string;
  int number;
  float floats;
  ValueType vt;
  /* seems that generated parser do not support unique_ptr */
  Value* p_value; 
  Attribute* attribute;
  CompareOp op;
  Operand* compare_operand;
  PredicateLeaf* condition_item;
  Predicate* conditions;
}

%token <number> NUMBER
%token <floats> FLOAT 
%token <string> ID
%token <string> STRING

%type   <vt> type
%type   <p_value> value
%type   <op> compare_op
%type   <attribute> attribute
%type   <compare_operand> condition_operand
%type   <condition_item> condition_item
%type   <conditions> conditions
%%
/* production list */
command: /* starts here */
      /* empty */
    | exit
    | help
    | show_tables

    | drop_table
    | desc_table
    | drop_index   
    | create_index

    | create_table
    
    | insert
    | update
    | delete
    | select  
      ;
exit: 
    EXIT SEMICOLON {
        CONTEXT->query = new Exit();
    };

help: 
    HELP SEMICOLON {
        CONTEXT->query = new Help();
    };

show_tables: 
    SHOW TABLES SEMICOLON {
        CONTEXT->query = new ShowTables();
    };

drop_table:
    DROP TABLE ID SEMICOLON {
        CONTEXT->query = new DropTable($3);
        free($3);
    };

desc_table:
    DESC ID SEMICOLON {
        CONTEXT->query = new DescTable($2);
        free($2);
    };

drop_index:
    DROP INDEX ID SEMICOLON {
        CONTEXT->query = new DropIndex($3);
        free($3);
    };
  
/* create index index_name on table(attribute) */
create_index:
    CREATE INDEX ID ON ID LBRACE ID RBRACE SEMICOLON {
        CONTEXT->query = new CreateIndex($3, $5, $7);

        free($3); free($5); free($7);
    };

/* create table table_name LBRACE schema RBRACE*/
create_table:
	CREATE TABLE ID { CONTEXT->query = new CreateTable($3); } LBRACE schema RBRACE SEMICOLON {
        free($3);
    };
	
schema:
	schema_item { }
	|
	schema COMMA schema_item {}
	;
schema_item:
    ID type {
        dynamic_cast<CreateTable*>(CONTEXT->query)->add_schema({$1, $2});
        free($1);
    };
type: 
    STRING_T {
        $$ = ValueType::VT_STRING;
    }
    |
    FLOAT_T {

        $$ = ValueType::VT_FLOAT;
    }
    |
    INT_T {
        $$ = ValueType::VT_INT;
    }
    |
    DATE_T {
        $$ = ValueType::VT_DATE;
    }
    ;
/* insert into table_name values (v1, v2 ...) */
insert:
    INSERT INTO ID { CONTEXT->query = new Insert($3); } insert_attr_part VALUES LBRACE insert_value_list RBRACE SEMICOLON{
        free($3);
    };
insert_attr_part:
	/* may be empty */
	|
	LBRACE insert_attr_list RBRACE {}
	;

insert_attr_list: 
	ID { 
        dynamic_cast<Insert*>(CONTEXT->query)->add_attribute($1);
        free($1);
    }
	|
	insert_attr_list COMMA ID {
        dynamic_cast<Insert*>(CONTEXT->query)->add_attribute($3);
        free($3);
    }
	; 
insert_value_list:  
	value { 
        dynamic_cast<Insert*>(CONTEXT->query)->add_value(std::unique_ptr<Value>($1)); 
    }
	| 
	insert_value_list COMMA value { 
        dynamic_cast<Insert*>(CONTEXT->query)->add_value(std::unique_ptr<Value>($3)); 
    }
	;

value:
	NUMBER { 
        $$ = new Integer($1); 
    }
	|
	FLOAT {
        $$ = new Float($1); 
    }
	|
	STRING {
        $$ = new String($1); 
        free($1);
    }
	;

/* update table_name SET a = b, c = d WHERE a = c AND b = d*/ 
update:
    UPDATE ID { CONTEXT->query = new Update($2); } SET update_set_list WHERE conditions SEMICOLON {
        dynamic_cast<Update*>(CONTEXT->query)->set_conditions(std::unique_ptr<Predicate>($7));
        free($2);
    };
update_set_list:
    update_set_item {} 
    |
    update_set_list COMMA update_set_item {}
    ;
update_set_item:
    ID EQ value {
        dynamic_cast<Update*>(CONTEXT->query)->add_set_item(std::make_unique<UpdateSetValue>($1, std::unique_ptr<Value>($3)));
        free($1);
    }
    |
    ID EQ ID {
        dynamic_cast<Update*>(CONTEXT->query)->add_set_item(std::make_unique<UpdateSetAttr>($1, $3));
        free($1); free($3);
    };
    
conditions:
    condition_item { $$ = $1; }
    |
    conditions AND conditions {
        $$ = new PredicateAnd(
            std::unique_ptr<Predicate>($1),
            std::unique_ptr<Predicate>($3)
        );
    }
    |
    conditions OR conditions {
        $$ = new PredicateOr(
            std::unique_ptr<Predicate>($1),
            std::unique_ptr<Predicate>($3)
        );
    }
    |
    NOT conditions {
        $$ = new PredicateNot (
            std::unique_ptr<Predicate>($2)
        );
    }
    |
    LBRACE conditions RBRACE {
        $$ = $2;
    };
condition_item: 
    condition_operand compare_op condition_operand {
        $$ = new PredicateLeaf(
            $2,
            std::unique_ptr<Operand>($1),
            std::unique_ptr<Operand>($3)
        );
    };
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
    attribute { $$ = new OperandAttr(std::unique_ptr<Attribute>($1)); }
    |
    value { $$ = new OperandVal(std::unique_ptr<Value>($1)); }
    ;
attribute:
    ID DOT ID {
        $$ = new Attribute($3, $1);
        free($1); free($3);
    }
    |
    ID { 
        $$ = new Attribute($1);
        free($1); 
    }
    ;
/* delete from table [where conditions]? */
delete:
    DELETE FROM ID WHERE conditions SEMICOLON {
        CONTEXT->query = new Delete(
            $3,
            std::unique_ptr<Predicate>($5) 
        );
        free($3);
    };
/* select attr_list from table where */
select:
	SELECT { CONTEXT->query = new Select(); } sel_list FROM from_list select_where SEMICOLON {

    }
    ;
select_where:
    /* empty */
    |
    WHERE conditions {
        dynamic_cast<Select*>(CONTEXT->query)->set_conditions(std::unique_ptr<Predicate>($2));
    };
sel_list:
    sel_list_item {}
    |
    sel_list COMMA sel_list_item {}
    ;

sel_list_item:
	STAR {
        dynamic_cast<Select*>(CONTEXT->query)->add_sel_attribute(std::make_unique<Attribute>("*"));
    }
	|
	ID DOT STAR { 
        dynamic_cast<Select*>(CONTEXT->query)->add_sel_attribute(std::make_unique<Attribute>("*", $1));
        free($1); 
    }
	|
    attribute {
        dynamic_cast<Select*>(CONTEXT->query)->add_sel_attribute(std::unique_ptr<Attribute>($1));
    }
	;
from_list:
    ID {
        dynamic_cast<Select*>(CONTEXT->query)->add_from_relation($1);
        free($1);
    }
    |
    ID COMMA ID { 
        dynamic_cast<Select*>(CONTEXT->query)->add_from_relation($1);
        dynamic_cast<Select*>(CONTEXT->query)->add_from_relation($3);
        free($1); free($3);
    }
    ;
%%

extern void scan_string(const char *str, yyscan_t scanner);

int sql_parse(const char *s, ParserContext& context) {
    memset(&context, 0, sizeof(ParserContext));
	yyscan_t scanner;
	yylex_init_extra(&context, &scanner);
	scan_string(s, scanner);
	int result = yyparse(scanner);
	yylex_destroy(scanner);
	return result;
}