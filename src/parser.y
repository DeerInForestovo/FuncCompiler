%{
#include <string>
#include <iostream>
#include <map>
#include "ast.hpp"
#include "definition.hpp"
#include "parser.hpp"
#include "parsed_type.hpp"

std::map<std::string, definition_data_ptr> defs_data;
std::map<std::string, definition_defn_ptr> defs_defn;

extern yy::parser::symbol_type yylex();
extern int yylineno;
int parser_error_cnt = 0;
void yyerror(const std::string &msg) {
    std::cerr << "Parser error at line " << yylineno << ": " << msg << std::endl;
    ++parser_error_cnt;
}
%}

%left OR
%left AND
%left BITOR
%left XOR
%left BITAND
%nonassoc EQ NEQ
%nonassoc LT GT LEQ GEQ
%left LMOVE RMOVE
%right NEGATE BITNOT NOT
%left PLUS MINUS
%left TIMES DIVIDE BMOD
%left CONNECT
%left INDEX

%token <float> FLOATNUMBER
%token <int> INTEGER
%token <char> CHARINSTANCE
%token <std::string> STRINGINSTANCE
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
%token COLON
%token ARROW
%token BIND
%token EQUAL
%token <std::string> LID
%token <std::string> UID

%language "c++"
%define api.value.type variant
%define api.token.constructor

%type <std::vector<std::string>> lowercaseParams
%type <std::vector<branch_ptr>> branches branchesDo
%type <std::vector<constructor_ptr>> constructors
%type <action_ptr> action bind
%type <std::vector<action_ptr>> actions actionOrBinds
%type <std::vector<parsed_type_ptr>> typeList
%type <parsed_type_ptr> type nonArrowType typeListElement
%type <ast_ptr> aAdd aMul case app appBase appIndex appConn appUniop aOr aAnd aBitor aXor aBitand aCmpeq aCmp aMove list
%type <definition_data_ptr> data 
%type <definition_defn_ptr> defn
%type <branch_ptr> branch branchDo
%type <pattern_ptr> pattern
%type <constructor_ptr> constructor
%type <std::vector<ast_ptr>> commaTermList colonTermList

%start program

%%

program
    : definitions { }
    ;

definitions
    : definitions definition { }
    | definition { }
    ;

definition
    : defn { auto name = $1->name; defs_defn[name] = std::move($1); }
    | data { auto name = $1->name; defs_data[name] = std::move($1); }
    ;

defn
    : DEFN LID lowercaseParams EQUAL OCURLY aOr CCURLY
        { $$ = definition_defn_ptr(
            new definition_defn(std::move($2), std::move($3), std::move($6))); }
    | DEFN LID lowercaseParams EQUAL DO OCURLY actions CCURLY 
        { $$ = definition_defn_ptr(
            new definition_defn(std::move($2), std::move($3), ast_ptr(new ast_do(std::move($7))))); }
    | DEFN LID lowercaseParams EQUAL OCURLY aOr error { $$ = definition_defn_ptr(
            new definition_defn(std::move($2), std::move($3), std::move($6)));
        yyerror("Unmatched '{'."); }
    | DEFN LID lowercaseParams EQUAL DO OCURLY actions error { $$ = definition_defn_ptr(
            new definition_defn(std::move($2), std::move($3), ast_ptr(new ast_do(std::move($7)))));
        yyerror("Unmatched '{'."); }
    | DEFN UID lowercaseParams EQUAL OCURLY aOr CCURLY
        { $$ = definition_defn_ptr(
            new definition_defn(std::move($2), std::move($3), std::move($6)));
        yyerror("Named a function with UID."); }
    | DEFN UID lowercaseParams EQUAL DO OCURLY actions CCURLY 
        { $$ = definition_defn_ptr(
            new definition_defn(std::move($2), std::move($3), ast_ptr(new ast_do(std::move($7)))));
        yyerror("Named a function with UID."); }
    ;

branchesDo
    : branchesDo branchDo { $$ = std::move($1); $$.push_back(std::move($2)); }
    | branchDo { $$ = std::vector<branch_ptr>(); $$.push_back(std::move($1));}
    ;

branchDo
    : pattern ARROW DO OCURLY actions CCURLY
        { $$ = branch_ptr(new branch(std::move($1), ast_ptr(new ast_do(std::move($5))))); }
    | pattern ARROW DO OCURLY actions error { $$ = branch_ptr(new branch(std::move($1), ast_ptr(new ast_do(std::move($5))))); yyerror("Unmatched '{'."); }
    ;

actions
    : actionOrBinds action { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

actionOrBinds
    : %empty { $$ = std::vector<action_ptr>(); }
    | actionOrBinds action { $$ = std::move($1); $$.push_back(std::move($2)); }
    | actionOrBinds bind { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

bind
    : DEFN LID BIND action { $$ = std::move($4); $$->bind_name = std::move($2); }

action
    : OCURLY aOr CCURLY { $$ = action_ptr(new action_exec(std::move($2))); }
    | RETURN OCURLY aOr CCURLY { $$ = action_ptr(new action_return(std::move($3))); }
    | CASE aOr OF OCURLY branchesDo CCURLY 
        { $$ = action_ptr(new action_exec(ast_ptr(new ast_case(std::move($2), std::move($5))))); }
    | OCURLY error CCURLY { yyerror("Illegal expr."); $$ = action_ptr(new action_exec(ast_ptr(new ast_int(0)))); }
    | RETURN OCURLY error CCURLY { yyerror("Illegal expr."); $$ = action_ptr(new action_return(ast_ptr(new ast_int(0)))); }
    ;

lowercaseParams
    : %empty { $$ = std::vector<std::string>(); }
    | lowercaseParams LID { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

aOr
    : aOr OR aAnd { $$ = ast_ptr(new ast_binop(OR, std::move($1), std::move($3))); }
    | aOr OR error { yyerror("Missing right operand of ||."); $$ = ast_ptr(new ast_int(0)); }
    | aAnd { $$ = std::move($1); }
    ;

aAnd
    : aAnd AND aBitor { $$ = ast_ptr(new ast_binop(AND, std::move($1), std::move($3))); }
    | aAnd AND error { yyerror("Missing right operand of &&."); $$ = ast_ptr(new ast_int(0)); }
    | aBitor { $$ = std::move($1); }
    ;

aBitor
    : aBitor BITOR aXor { $$ = ast_ptr(new ast_binop(BITOR, std::move($1), std::move($3))); }
    | aBitor BITOR error { yyerror("Missing right operand of |."); $$ = ast_ptr(new ast_int(0)); }
    | aXor { $$ = std::move($1); }
    ;

aXor
    : aXor XOR aBitand { $$ = ast_ptr(new ast_binop(XOR, std::move($1), std::move($3))); }
    | aXor XOR error { yyerror("Missing right operand of ^."); $$ = ast_ptr(new ast_int(0)); }
    | aBitand { $$ = std::move($1); }
    ;

aBitand
    : aBitand BITAND aCmpeq { $$ = ast_ptr(new ast_binop(BITAND, std::move($1), std::move($3))); }
    | aBitand BITAND error { yyerror("Missing right operand of &."); $$ = ast_ptr(new ast_int(0)); }
    | aCmpeq { $$ = std::move($1); }
    ;

aCmpeq
    : aCmpeq EQ aCmp { $$ = ast_ptr(new ast_binop(EQ, std::move($1), std::move($3))); }
    | aCmpeq EQ error { yyerror("Missing right operand of ==."); $$ = ast_ptr(new ast_int(0)); }
    | aCmpeq NEQ aCmp { $$ = ast_ptr(new ast_binop(NEQ, std::move($1), std::move($3))); }
    | aCmpeq NEQ error { yyerror("Missing right operand of !=."); $$ = ast_ptr(new ast_int(0)); }
    | aCmp { $$ = std::move($1); }
    ;

aCmp
    : aCmp LT aMove { $$ = ast_ptr(new ast_binop(LT, std::move($1), std::move($3))); }
    | aCmp LT error { yyerror("Missing right operand of <."); $$ = ast_ptr(new ast_int(0)); }
    | aCmp GT aMove { $$ = ast_ptr(new ast_binop(GT, std::move($1), std::move($3))); }
    | aCmp GT error { yyerror("Missing right operand of >."); $$ = ast_ptr(new ast_int(0)); }
    | aCmp LEQ aMove { $$ = ast_ptr(new ast_binop(LEQ, std::move($1), std::move($3))); }
    | aCmp LEQ error { yyerror("Missing right operand of <=."); $$ = ast_ptr(new ast_int(0)); }
    | aCmp GEQ aMove { $$ = ast_ptr(new ast_binop(GEQ, std::move($1), std::move($3))); }
    | aCmp GEQ error { yyerror("Missing right operand of >=."); $$ = ast_ptr(new ast_int(0)); }
    | aMove { $$ = std::move($1); }
    ;

aMove
    : aMove LMOVE aAdd { $$ = ast_ptr(new ast_binop(LMOVE, std::move($1), std::move($3))); }
    | aMove LMOVE error { yyerror("Missing right operand of <<."); $$ = ast_ptr(new ast_int(0)); }
    | aMove RMOVE aAdd { $$ = ast_ptr(new ast_binop(RMOVE, std::move($1), std::move($3))); }
    | aMove RMOVE error { yyerror("Missing right operand of >>."); $$ = ast_ptr(new ast_int(0)); }
    | aAdd { $$ = std::move($1); }
    ;

aAdd
    : aAdd PLUS aMul { $$ = ast_ptr(new ast_binop(PLUS, std::move($1), std::move($3))); }
    | aAdd PLUS error { yyerror("Missing right operand of +."); $$ = ast_ptr(new ast_int(0)); }
    | aAdd MINUS aMul { $$ = ast_ptr(new ast_binop(MINUS, std::move($1), std::move($3))); }
    | aAdd MINUS error { yyerror("Missing right operand of -."); $$ = ast_ptr(new ast_int(0)); }
    | aMul { $$ = std::move($1); }
    ;

aMul
    : aMul TIMES app { $$ = ast_ptr(new ast_binop(TIMES, std::move($1), std::move($3))); }
    | aMul TIMES error { yyerror("Missing right operand of *."); $$ = ast_ptr(new ast_int(0)); }
    | aMul DIVIDE app { $$ = ast_ptr(new ast_binop(DIVIDE, std::move($1), std::move($3))); }
    | aMul DIVIDE error { yyerror("Missing right operand of /."); $$ = ast_ptr(new ast_int(0)); }
    | aMul BMOD app { $$ = ast_ptr(new ast_binop(BMOD, std::move($1), std::move($3))); }
    | aMul BMOD error { yyerror("Missing right operand of %."); $$ = ast_ptr(new ast_int(0)); }
    | app %prec MINUS { $$ = std::move($1); }
    ;

app
    : app appUniop { $$ = ast_ptr(new ast_app(std::move($1), std::move($2))); }
    | appUniop { $$ = std::move($1); }
    ;

appUniop  // Apply union-operators
    : MINUS appConn %prec NEGATE { $$ = ast_ptr(new ast_uniop(NEGATE, std::move($2))); }
    | NOT appConn { $$ = ast_ptr(new ast_uniop(NOT, std::move($2))); }
    | BITNOT appConn { $$ = ast_ptr(new ast_uniop(BITNOT, std::move($2))); }
    | appConn { $$ = std::move($1); }

appConn  // Connect two lists
    : appConn CONNECT appIndex { $$ = ast_ptr(new ast_binop(CONN, std::move($1), std::move($3))); }
    | appIndex { $$ = std::move($1); }

appIndex  // Index from a list
    : appIndex INDEX appBase { $$ = ast_ptr(new ast_binop(INDEX, std::move($1), std::move($3))); }
    | appBase { $$ = std::move($1); }

appBase
    : FLOATNUMBER { $$ = ast_ptr(new ast_float($1)); }
    | INTEGER { $$ = ast_ptr(new ast_int($1)); }
    | STRINGINSTANCE { $$ = ast_ptr(new ast_list(std::move($1))); }  // string = list* char
    | CHARINSTANCE { $$ = ast_ptr(new ast_char($1)); }
    | LID { $$ = ast_ptr(new ast_lid(std::move($1))); }
    | UID { $$ = ast_ptr(new ast_uid(std::move($1))); }
    | OPAREN aOr CPAREN { $$ = std::move($2); }
    | case { $$ = std::move($1); }
    | list { $$ = std::move($1); }
    | OPAREN error CPAREN { yyerror("Illegal expression."); $$ = ast_ptr(new ast_int(0)); }
    | OSQUARE error CSQUARE { yyerror("Illegal expression."); $$ = ast_ptr(new ast_int(0)); }
    ;

case
    : CASE aOr OF OCURLY branches CCURLY 
        { $$ = ast_ptr(new ast_case(std::move($2), std::move($5))); }
    | CASE aOr OF OCURLY branches error { $$ = ast_ptr(new ast_case(std::move($2), std::move($5))); yyerror("Unmatched '{'."); }
    ;

branches
    : branches branch { $$ = std::move($1); $$.push_back(std::move($2)); }
    | branch { $$ = std::vector<branch_ptr>(); $$.push_back(std::move($1));}
    ;

branch
    : pattern ARROW OCURLY aOr CCURLY
        { $$ = branch_ptr(new branch(std::move($1), std::move($4))); }
    | pattern ARROW OCURLY aOr error { $$ = branch_ptr(new branch(std::move($1), std::move($4))); yyerror("Unmatched '{'."); }
    ;

pattern
    : LID { $$ = pattern_ptr(new pattern_var(std::move($1))); }
    | UID lowercaseParams
        { $$ = pattern_ptr(new pattern_constr(std::move($1), std::move($2))); }
    ;

data
    : DATA UID lowercaseParams EQUAL OCURLY constructors CCURLY
        { $$ = definition_data_ptr(new definition_data(std::move($2), std::move($3), std::move($6))); }
    ;

constructors
    : constructors COMMA constructor { $$ = std::move($1); $$.push_back(std::move($3)); }
    | constructor
        { $$ = std::vector<constructor_ptr>(); $$.push_back(std::move($1)); }
    ;

constructor
    : UID typeList
        { $$ = constructor_ptr(new constructor(std::move($1), std::move($2))); }
    ;

type
    : nonArrowType ARROW type { $$ = parsed_type_ptr(new parsed_type_arr(std::move($1), std::move($3))); }
    | nonArrowType { $$ = std::move($1); }
    ;

nonArrowType
    : UID typeList { $$ = parsed_type_ptr(new parsed_type_app(std::move($1), std::move($2))); }
    | LID { $$ = parsed_type_ptr(new parsed_type_var(std::move($1))); }
    | OPAREN type CPAREN { $$ = std::move($2); }
    ;

typeListElement
    : OPAREN type CPAREN { $$ = std::move($2); }
    | UID { $$ = parsed_type_ptr(new parsed_type_app(std::move($1), {})); }
    | LID { $$ = parsed_type_ptr(new parsed_type_var(std::move($1))); }
    ;

typeList
    : %empty { $$ = std::vector<parsed_type_ptr>(); }
    | typeList typeListElement { $$ = std::move($1); $$.push_back(std::move($2)); }
    ; 

list
    : OSQUARE CSQUARE { $$ = ast_ptr(new ast_list(std::vector<ast_ptr>())); }
    | OSQUARE commaTermList CSQUARE { $$ = ast_ptr(new ast_list(std::move($2))); }
    | OSQUARE colonTermList CSQUARE { $$ = ast_ptr(new ast_list_colon(std::move($2))); }
    | OSQUARE commaTermList error { $$ = ast_ptr(new ast_list(std::move($2))); yyerror("Unmatched '['."); }
    ;

commaTermList
    : aOr { $$ = std::vector<ast_ptr>(); $$.push_back(std::move($1)); }
    | commaTermList COMMA aOr { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;

colonTermList
    : aOr COLON aOr { $$ = std::vector<ast_ptr>(); $$.push_back(std::move($1)); $$.push_back(std::move($3)); }
    | colonTermList COLON aOr { $$ = std::move($1); $$.push_back(std::move($3)); }
    ;