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
%token <int> INT
%token DEFN
%token DATA
%token CASE
%token OF
%token OCURLY
%token CCURLY
%token OPAREN
%token CPAREN
%token COMMA
%token ARROW
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
%type <ast_ptr> aAdd aMul case app appBase
%type <definition_ptr> definition defn data 
%type <branch_ptr> branch
%type <pattern_ptr> pattern
%type <constructor_ptr> constructor

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
    : DEFN LID lowercaseParams EQUAL OCURLY aAdd CCURLY
        { $$ = definition_ptr(
            new definition_defn(std::move($2), std::move($3), std::move($6))); }
    ;

lowercaseParams
    : %empty { $$ = std::vector<std::string>(); }
    | lowercaseParams LID { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

uppercaseParams
    : %empty { $$ = std::vector<std::string>(); }
    | uppercaseParams UID { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

aAdd
    : aAdd PLUS aMul { $$ = ast_ptr(new ast_binop(PLUS, std::move($1), std::move($3))); }
    | aAdd MINUS aMul { $$ = ast_ptr(new ast_binop(MINUS, std::move($1), std::move($3))); }
    | aMul { $$ = std::move($1); }
    ;

aMul
    : aMul TIMES app { $$ = ast_ptr(new ast_binop(TIMES, std::move($1), std::move($3))); }
    | aMul DIVIDE app { $$ = ast_ptr(new ast_binop(DIVIDE, std::move($1), std::move($3))); }
    | app { $$ = std::move($1); }
    ;

app
    : app appBase { $$ = ast_ptr(new ast_app(std::move($1), std::move($2))); }
    | appBase { $$ = std::move($1); }
    ;

appBase
    : INT { $$ = ast_ptr(new ast_int($1)); }
    | LID { $$ = ast_ptr(new ast_lid(std::move($1))); }
    | UID { $$ = ast_ptr(new ast_uid(std::move($1))); }
    | OPAREN aAdd CPAREN { $$ = std::move($2); }
    | case { $$ = std::move($1); }
    ;

case
    : CASE aAdd OF OCURLY branches CCURLY 
        { $$ = ast_ptr(new ast_case(std::move($2), std::move($5))); }
    ;

branches
    : branches branch { $$ = std::move($1); $1.push_back(std::move($2)); }
    | branch { $$ = std::vector<branch_ptr>(); $$.push_back(std::move($1));}
    ;

branch
    : pattern ARROW OCURLY aAdd CCURLY
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
