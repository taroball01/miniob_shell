/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_WORKSPACES_MINIOB_SHELL_SRC_SQL_YACC_TAB_H_INCLUDED
# define YY_YY_WORKSPACES_MINIOB_SHELL_SRC_SQL_YACC_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    EXIT = 258,
    SEMICOLON = 259,
    HELP = 260,
    SHOW = 261,
    TABLES = 262,
    DROP = 263,
    TABLE = 264,
    DESC = 265,
    INDEX = 266,
    CREATE = 267,
    ON = 268,
    LBRACE = 269,
    RBRACE = 270,
    COMMA = 271,
    INT_T = 272,
    STRING_T = 273,
    FLOAT_T = 274,
    DATE_T = 275,
    INSERT = 276,
    INTO = 277,
    VALUES = 278,
    UPDATE = 279,
    SET = 280,
    AND = 281,
    OR = 282,
    NOT = 283,
    EQ = 284,
    LE = 285,
    NE = 286,
    LT = 287,
    GE = 288,
    GT = 289,
    DOT = 290,
    DELETE = 291,
    FROM = 292,
    SELECT = 293,
    STAR = 294,
    WHERE = 295,
    NUMBER = 296,
    FLOAT = 297,
    ID = 298,
    STRING = 299
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 66 "/workspaces/miniob_shell/src/sql/yacc.y"
  
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

#line 116 "/workspaces/miniob_shell/src/sql/yacc.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (void *scanner);

#endif /* !YY_YY_WORKSPACES_MINIOB_SHELL_SRC_SQL_YACC_TAB_H_INCLUDED  */
