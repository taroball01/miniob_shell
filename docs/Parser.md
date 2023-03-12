## Lexer & Parser Design

使用 `flex` 和 `bison` 做词法和语法分析。`flex`可以生成c++词法分析器，但其文档中注明：

> **IMPORTANT**: the present form of the scanning class is *experimental* and may change considerably between major releases.

所以本项目生成c语言版的分析器源文件，但可以使用c++编译器编译，只需要在cmake中需为文件添加属性：

```cmake
set_source_files_properties(filename.c PROPERTIES LANGUAGE CXX )
```

#### Interface

接口设计:

```c
int sql_parse(const char* sql, ParserContext& context);
```



为了使每一次分析都是独立的，而不依赖于全局变量，可以定义生成可重入的解析器。

`flex`·: `%option reentrant` , [相关文档](https://www.cs.virginia.edu/~cr4bd/flex-manual/Reentrant-Detail.html#Reentrant-Detail)

`bison`:`%define api.pure full` [%define api.pure purity](https://www.gnu.org/software/bison/manual/html_node/_0025define-Summary.html#index-_0025define-api_002epure-1)

这样会为flex生成的大部分函数引入额外的`yyscan_t`参数，见[Flex 2.6.3: Extra Reentrant Argument ](https://www.cs.virginia.edu/~cr4bd/flex-manual/Extra-Reentrant-Argument.html#Extra-Reentrant-Argument)，用于传入可选的extra参数。

同时应当从字符串中获取输入而非文件，需要在lexer.l中定义：

```c
void scan_string(const char *str, yyscan_t scanner) {
  yy_switch_to_buffer(yy_scan_string(str, scanner), scanner);
}
```

将输入的`buffer`由标准输入切换为以传入的`str`参数初始化的`buffer`。

具体使用方法：

```cpp
int sql_parse(const char* sql, ParserContext& context) {
    yyscan_t scanner;
    // store context in scanner
    yylex_init_extra(&context, &scanner);
    // switch buffer 
    scan_string(sql, scanner);
    // start parse
    int result = yyparse(scanner);
    // destroy extra parameter stored in scanner
    yylex_destroy(scanner);
    return result;
}
```

`yyparse`函数通过修改传入的`scanner`参数里存储的`context`对象，在解析语句时存储相应信息。

-----------------------

#### Lexer

`flex`文法模板：

```less
% {
/* definition part of c*/   
%}

/* ---- Options Part ---- */
/* noyywrap means if get EOF then won't change to a new input buffer */ 
%option noyywrap
/* http://westes.github.io/flex/manual/Bison-Bridge.html */
%option bison-bridge
/* https://www.cs.virginia.edu/~cr4bd/flex-manual/Reentrant-Detail.html#Reentrant-Detail */
%option reentrant

%%
/* Token here*/
%%
/* define c functions here */
void scan_string(const char *str, yyscan_t scanner) {
  yy_switch_to_buffer(yy_scan_string(str, scanner), scanner);
}
```

-----------------------

#### Parser

`parser`语法模板

```less
%{
/* definition part of c*/   
%}

%define api.pure full
%lex-param { yyscan_t scanner }
%parse-param { void *scanner }

%token ...
/* YYSTYPE */
%union {
    int number;
}

%%
/* production list */

%%

/* c functions */
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
```



