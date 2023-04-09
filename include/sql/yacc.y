%{
/* this part will copy to yacc.tab.c */
#include "sql/parser.h"

/* lexer.yy.h depends on yacc.tab.h, because of `YYSTYPE` */

// clang-format off
#include "sql/yacc.tab.h"
#include "sql/lexer.yy.h"
// clang-format on

#include "sql/query/select.h"
#include "sql/query/create_table.h"
#include "sql/query/insert.h"
#include "sql/query/delete.h"
using namespace query_process_engine;
#define CONTEXT query_process_engine::get_context(scanner)

%}

%code {
  int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner);
  void yyerror(YYLTYPE* yyllocp, yyscan_t unused, const char* msg);
}

%define api.pure full
%lex-param { yyscan_t scanner }
%parse-param { void *scanner }
%define parse.error verbose
%locations

%token  EXIT
        SELECT
        FROM
        SEMICOLON
        COMMA
        STAR 
        DOT
        WHERE
        SHOW
        TABLES
        DESC
        CREATE
        TABLE
        INT_T
        STRING_T
        DATE_T
        FLOAT_T
        INSERT
        INTO
        VALUES
        DELETE

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
        ID
        STRING
        
%left OR
%left AND
%right NOT
/* YYSTYPE */
%union {  
  int number;
  float floats;
  query_process_engine::CompareOp compare_op;
  query_process_engine::ValueType value_type;
}

%token <number> NUMBER
%token <floats> FLOAT 
%type <compare_op> compare_op
%type <value_type> value_type
%%
/* production list */
command: /* starts here */
  exit
  |
  select  
  |
  show_tables
  |
  desc_table
  |
  create_table
  | 
  insert
  |
  delete
  ;
  
exit:
  EXIT SEMICOLON { CONTEXT->query_ = std::make_unique<ExitCmd>(); };

show_tables:
  SHOW TABLES SEMICOLON { CONTEXT->query_ = std::make_unique<ShowTables>(); }
  ;

desc_table:
  DESC ID SEMICOLON { CONTEXT->query_ = std::make_unique<DescTable>(CONTEXT->pop_str()); }
  ;

create_table:
  CREATE TABLE ID { CONTEXT->query_ = std::make_unique<CreateTable>(CONTEXT->pop_str()); } LBRACE create_schema RBRACE SEMICOLON {}
  ;

create_schema:
  create_schema_item {}
  |
  create_schema COMMA create_schema_item {}
  ;

create_schema_item:
  ID value_type { 
    auto* query = CONTEXT->get_query<CreateTable>();
    query->append_schema_item(SchemaItem{"", CONTEXT->pop_str(), $2});
  }
  ;

value_type:
  INT_T { $$ = ValueType::VT_INT; }
  |
  STRING_T { $$ = ValueType::VT_STRING; }
  | 
  DATE_T { $$ = ValueType::VT_DATE; }
  |
  FLOAT_T { $$ = ValueType::VT_FLOAT; }
  ;

insert:
  INSERT INTO ID {  CONTEXT->query_ = std::make_unique<InsertQuery>(CONTEXT->pop_str()); } VALUES LBRACE value_list RBRACE SEMICOLON;

value_list:
  value { 
    auto* insert = CONTEXT->get_query<InsertQuery>();
    insert->append_value(CONTEXT->pop_value()); 
  }
  |
  value_list COMMA value { 
    auto* insert = CONTEXT->get_query<InsertQuery>();
    insert->append_value(CONTEXT->pop_value()); 
  } 
  ;
  
delete:
  DELETE FROM ID { CONTEXT->query_ = std::make_unique<DeleteQuery>(CONTEXT->pop_str()); } WHERE conditions SEMICOLON {
    auto* del = CONTEXT->get_query<DeleteQuery>();
    del->set_condition(CONTEXT->pop_predicate());
  }
  ;


select:
  SELECT { CONTEXT->query_ = std::make_unique<SelectQuery>(); } sel_list FROM from_list where_clause SEMICOLON {}
  ;

sel_list:
  sel_list_item {}
  |
  sel_list COMMA sel_list_item {}
  ;

sel_list_item:
  STAR { 
    auto* query = CONTEXT->get_query<SelectQuery>();
    query->add_attribute(Attribute{"*"});
  }
  | 
  attribute {
    auto* query = CONTEXT->get_query<SelectQuery>();
    query->add_attribute(CONTEXT->pop_attr());
  }
  ;

attribute:
  ID DOT ID { 
    std::string attr = CONTEXT->pop_str();
    std::string rel = CONTEXT->pop_str();
    CONTEXT->attr_stack_.emplace_back(attr, rel); 
  }
  |
  ID {
    std::string attr = CONTEXT->pop_str();
    CONTEXT->attr_stack_.emplace_back(attr);
  }
  ;
from_list:
  ID {
    auto* query = CONTEXT->get_query<SelectQuery>();
    query->add_relation(CONTEXT->pop_str());
  }
  |
  from_list COMMA ID {
    auto* query = CONTEXT->get_query<SelectQuery>();
    query->add_relation(CONTEXT->pop_str());
  }
  ;

where_clause:
  /* empty */
  |
  WHERE conditions {
    auto* query = CONTEXT->get_query<SelectQuery>();
    query->set_condition(CONTEXT->pop_predicate());
  }
  ;

conditions:
  condition_operand compare_op condition_operand {
    auto r_child = CONTEXT->pop_operand();
    auto l_child = CONTEXT->pop_operand();
    CONTEXT->pred_stack_.emplace_back(std::make_unique<PredicateLeaf>($2, std::move(l_child), std::move(r_child)));
  }
  |
  LBRACE conditions RBRACE {
    // do nothing, conditions is still in stack top
  }
  |
  NOT conditions {
    auto child = CONTEXT->pop_predicate();
    CONTEXT->pred_stack_.emplace_back(std::make_unique<PredicateNot>(std::move(child)));
  }
  |
  conditions AND conditions {
    auto r_child = CONTEXT->pop_predicate();
    auto l_child = CONTEXT->pop_predicate();
    CONTEXT->pred_stack_.emplace_back(std::make_unique<PredicateAnd>(std::move(l_child), std::move(r_child)));
  }
  | 
  conditions OR conditions {
    auto r_child = CONTEXT->pop_predicate();
    auto l_child = CONTEXT->pop_predicate();
    CONTEXT->pred_stack_.emplace_back(std::make_unique<PredicateOr>(std::move(l_child), std::move(r_child)));
  }
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
  attribute {
    CONTEXT->operand_stack_.emplace_back(std::make_unique<OperandAttr>(CONTEXT->pop_attr()));
  }
  |
  value {
    CONTEXT->operand_stack_.emplace_back(std::make_unique<OperandVal>(CONTEXT->pop_value()));
  }
  ;

value:
  NUMBER { 
    CONTEXT->value_stack_.emplace_back(std::make_unique<Integer>($1));
  }
  |
  FLOAT {
    CONTEXT->value_stack_.emplace_back(std::make_unique<Float>($1));
  }
  |
  STRING {
    CONTEXT->value_stack_.emplace_back(std::make_unique<String>(CONTEXT->pop_str().data()));
  }
  ;
%%


