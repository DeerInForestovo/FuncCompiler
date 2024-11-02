%require "3.2"

%{
#include <string>
#include <iostream>
#include "ast.hpp"
#include "parser.tab.hh"

std::vector<definition_ptr> program;
extern yy::parser::symbol_type yylex();

%}

%token PLUS
%token TIMES
%token MINUS
%token DIVIDE
%token BMOD
%token BITAND
%token BITOR
%token BITNOT
%token AND
%token OR
%token NOT
%token XOR
%token LT
%token GT
%token LEQ
%token GEQ
%token EQ
%token NEQ
%token LMOVE
%token RMOVE
%token <float> FLOATNUMBER
%token <int> INTEGER
%token TRUE
%token FALSE
%token <std::string> STRINGINSTANCE
%token INT
%token FLOAT
%token BOOL
%token STRING
%token DEFN
%token DATA
%token CASE
%token OF
%token DO
%token RETURN
%token OCURLY
%token CCURLY
%token OPAREN
%token CPAREN
%token OSQUARE
%token CSQUARE
%token COMMA
%token ARROW
%token BIND
%token EQUAL
%token <std::string> LID
%token <std::string> UID

%language "c++"
%define api.value.type variant
%define api.token.constructor

%type <std::vector<std::string>> lowercaseParams uppercaseParams
%type <std::vector<definition_ptr>> program definitions
%type <std::vector<branch_ptr>> branches
%type <std::vector<constructor_ptr>> constructors
%type <std::vector<action_ptr>> actions
%type <ast_ptr> aAdd aMul case app appBase tuple list aOr aAnd aBitor aXor aBitand aCmpeq aCmp aMove
%type <definition_ptr> definition defn data 
%type <branch_ptr> branch
%type <pattern_ptr> pattern
%type <constructor_ptr> constructor
%type <action_ptr> action actionBase
%type <std::vector<ast_ptr>> termlist

%start program

%%

program
    : definitions { program = std::move($1); }
    ;

definitions
    : definitions definition { $$ = std::move($1); $$.push_back(std::move($2)); }
    | definition { $$ = std::vector<definition_ptr>(); $$.push_back(std::move($1)); }
    ;

definition
    : defn { $$ = std::move($1); }
    | data { $$ = std::move($1); }
    ;

defn
    : DEFN LID lowercaseParams EQUAL OCURLY aOr CCURLY
        { $$ = definition_ptr(
            new definition_defn(std::move($2), std::move($3), std::move($6))); }
    | DEFN LID lowercaseParams EQUAL DO OCURLY actions CCURLY 
        { $$ = definition_ptr(
            new definition_defn_action(std::move($2), std::move($3), std::move($7))); }
    ;

actions
    : actions action { $$ = std::move($1); $$.push_back(std::move($2)); }
    | action { $$ = std::vector<action_ptr>(); $$.push_back(std::move($1)); }
    ;

action
    : actionBase { $$ = std::move($1); }
    | DEFN LID BIND actionBase { $$ = action_ptr(new action_bind(std::move($2), std::move($4))); }
    ;

actionBase
    : OCURLY aOr CCURLY { $$ = action_ptr(new action_exec(std::move($2))); }
    | RETURN OCURLY aOr CCURLY { $$ = action_ptr(new action_return(std::move($3))); }
    ;

lowercaseParams
    : %empty { $$ = std::vector<std::string>(); }
    | lowercaseParams LID { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

uppercaseParams
    : %empty { $$ = std::vector<std::string>(); }
    | uppercaseParams UID { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

aOr
    : aOr OR aAnd { $$ = ast_ptr(new ast_binop(OR, std::move($1), std::move($3))); }
    | aAnd { $$ = std::move($1); }
    ;

aAnd
    : aAnd AND aBitor { $$ = ast_ptr(new ast_binop(AND, std::move($1), std::move($3))); }
    | aBitor { $$ = std::move($1); }
    ;

aBitor
    : aBitor BITOR aXor { $$ = ast_ptr(new ast_binop(BITOR, std::move($1), std::move($3))); }
    | aXor { $$ = std::move($1); }
    ;

aXor
    : aXor XOR aBitand { $$ = ast_ptr(new ast_binop(XOR, std::move($1), std::move($3))); }
    | aBitand { $$ = std::move($1); }
    ;

aBitand
    : aBitand BITAND aCmpeq { $$ = ast_ptr(new ast_binop(BITAND, std::move($1), std::move($3))); }
    | aCmpeq { $$ = std::move($1); }
    ;

aCmpeq
    : aCmpeq EQ aCmp { $$ = ast_ptr(new ast_binop(EQ, std::move($1), std::move($3))); }
    | aCmpeq NEQ aCmp { $$ = ast_ptr(new ast_binop(NEQ, std::move($1), std::move($3))); }
    | aCmp { $$ = std::move($1); }
    ;

aCmp
    : aCmp LT aMove { $$ = ast_ptr(new ast_binop(LT, std::move($1), std::move($3))); }
    | aCmp GT aMove { $$ = ast_ptr(new ast_binop(GT, std::move($1), std::move($3))); }
    | aCmp LEQ aMove { $$ = ast_ptr(new ast_binop(LEQ, std::move($1), std::move($3))); }
    | aCmp GEQ aMove { $$ = ast_ptr(new ast_binop(GEQ, std::move($1), std::move($3))); }
    | aMove { $$ = std::move($1); }
    ;

aMove
    : aMove LMOVE aAdd { $$ = ast_ptr(new ast_binop(LMOVE, std::move($1), std::move($3))); }
    | aMove RMOVE aAdd { $$ = ast_ptr(new ast_binop(RMOVE, std::move($1), std::move($3))); }
    | aAdd { $$ = std::move($1); }
    ;

aAdd
    : aAdd PLUS aMul { $$ = ast_ptr(new ast_binop(PLUS, std::move($1), std::move($3))); }
    | aAdd MINUS aMul { $$ = ast_ptr(new ast_binop(MINUS, std::move($1), std::move($3))); }
    | aMul { $$ = std::move($1); }
    ;

aMul
    : aMul TIMES app { $$ = ast_ptr(new ast_binop(TIMES, std::move($1), std::move($3))); }
    | aMul DIVIDE app { $$ = ast_ptr(new ast_binop(DIVIDE, std::move($1), std::move($3))); }
    | aMul BMOD app { $$ = ast_ptr(new ast_binop(BMOD, std::move($1), std::move($3))); }
    | app { $$ = std::move($1); }
    ;

app
    : app appBase { $$ = ast_ptr(new ast_app(std::move($1), std::move($2))); }
    | appBase { $$ = std::move($1); }
    ;

appBase
    : FLOATNUMBER { $$ = ast_ptr(new ast_float($1)); }
    | INTEGER { $$ = ast_ptr(new ast_int($1)); }
    | STRINGINSTANCE { $$ = ast_ptr(new ast_string(std::move($1))); }
    | TRUE { $$ = ast_ptr(new ast_bool(true)); }
    | FALSE { $$ = ast_ptr(new ast_bool(false)); }
    | LID { $$ = ast_ptr(new ast_lid(std::move($1))); }
    | UID { $$ = ast_ptr(new ast_uid(std::move($1))); }
    | OPAREN aOr CPAREN { $$ = std::move($2); }
    | case { $$ = std::move($1); }
    | tuple { $$ = std::move($1); }
    | list { $$ = std::move($1); }
    | OSQUARE aOr CSQUARE { $$ = ast_ptr(new ast_index(std::move($2))); }
    | NOT { $$ = ast_ptr(new ast_uniop(NOT)); }
    | BITNOT { $$ = ast_ptr(new ast_uniop(BITNOT)); }
    ;

case
    : CASE aOr OF OCURLY branches CCURLY 
        { $$ = ast_ptr(new ast_case(std::move($2), std::move($5))); }
    ;

branches
    : branches branch { $$ = std::move($1); $$.push_back(std::move($2)); }
    | branch { $$ = std::vector<branch_ptr>(); $$.push_back(std::move($1));}
    ;

branch
    : pattern ARROW OCURLY aOr CCURLY
        { $$ = branch_ptr(new branch(std::move($1), std::move($4))); }
    ;

pattern
    : LID { $$ = pattern_ptr(new pattern_var(std::move($1))); }
    | UID lowercaseParams
        { $$ = pattern_ptr(new pattern_constr(std::move($1), std::move($2))); }
    ;

data
    : DATA UID EQUAL OCURLY constructors CCURLY
        { $$ = definition_ptr(new definition_data(std::move($2), std::move($5))); }
    ;

constructors
    : constructors COMMA constructor { $$ = std::move($1); $$.push_back(std::move($3)); }
    | constructor
        { $$ = std::vector<constructor_ptr>(); $$.push_back(std::move($1)); }
    ;

constructor
    : UID uppercaseParams
        { $$ = constructor_ptr(new constructor(std::move($1), std::move($2))); }
    ;

list
    : OSQUARE CSQUARE { $$ = ast_ptr(new ast_list(std::vector<ast_ptr>())); }
    | OSQUARE termlist COMMA CSQUARE { $$ = ast_ptr(new ast_list(std::move($2))); }
    ;

tuple
    : OPAREN termlist COMMA CPAREN { $$ = ast_ptr(new ast_tuple(std::move($2))); }
    ;

termlist
    : aOr { $$ = std::vector<ast_ptr>(); $$.push_back(std::move($1)); }
    | termlist COMMA aOr { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;