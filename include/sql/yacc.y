%{
/* this part will copy to yacc.tab.c */
#include "sql/parser.h"

/* lexer.yy.h depends on yacc.tab.h, because of `YYSTYPE` */
#include "sql/yacc.tab.h"
#include "sql/lexer.yy.h"

#include "sql/query/select.h"
#define CONTEXT get_context(scanner)
%}
/* */
%define api.pure full
%lex-param { yyscan_t scanner }
%parse-param { void *scanner }

%token  EXIT
        SELECT
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
        ID
        STRING
        
%left OR
%left AND
%right NOT
/* YYSTYPE */
%union {  
  int number;
  float floats;
  CompareOp compare_op;
}

%token <number> NUMBER
%token <floats> FLOAT 
%type <compare_op> compare_op;
%%
/* production list */
command: /* starts here */
  exit
  |
  select  
  ;
  
exit:
  EXIT SEMICOLON { CONTEXT->query_ = std::make_unique<ExitCmd>(); };

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
  ID DOT STAR { 
    auto* query = CONTEXT->get_query<SelectQuery>();
    query->add_attribute(Attribute{"*", CONTEXT->pop_str()});
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

