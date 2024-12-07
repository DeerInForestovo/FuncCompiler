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

#define REPORT_ERROR(fmt, args...) do { \
    fprintf(stderr, fmt, ##args); \
    fprintf(stderr, "\n"); \
} while (0);

%}

%left OR
%left AND
%left BITOR
%left XOR
%left BITAND
%nonassoc EQ NEQ
%nonassoc LT GT LEQ GEQ
%left LMOVE RMOVE
%right NEGATE
%left PLUS MINUS
%left TIMES DIVIDE BMOD

%token BITNOT
%token NOT
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

%type <std::vector<std::string>> lowercaseParams
%type <std::vector<branch_ptr>> branches
%type <std::vector<constructor_ptr>> constructors
// %type <action_ptr> action actionBase
// %type <std::vector<action_ptr>> actions
%type <std::vector<parsed_type_ptr>> typeList
%type <parsed_type_ptr> type nonArrowType typeListElement
%type <ast_ptr> aAdd aMul case app appBase tuple list aOr aAnd aBitor aXor aBitand aCmpeq aCmp aMove
%type <definition_data_ptr> data 
%type <definition_defn_ptr> defn
%type <branch_ptr> branch
%type <pattern_ptr> pattern
%type <constructor_ptr> constructor
%type <std::vector<ast_ptr>> termlist
%type <std::string> extendedUID

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
    // | DEFN LID lowercaseParams EQUAL DO OCURLY actions CCURLY 
    //     { $$ = definition_ptr(
    //         new definition_defn_action(std::move($2), std::move($3), std::move($7))); }
    | DEFN LID lowercaseParams EQUAL OCURLY aOr error { REPORT_ERROR("Unmatched '{'."); }
    // | DEFN LID lowercaseParams EQUAL DO OCURLY actions error { REPORT_ERROR("Unmatched '{'."); }
    | DEFN LID lowercaseParams EQUAL OCURLY error CCURLY { REPORT_ERROR("Illegal expr."); }
    ;

// actions
//     : actions action { $$ = std::move($1); $$.push_back(std::move($2)); }
//     | action { $$ = std::vector<action_ptr>(); $$.push_back(std::move($1)); }
//     ;

// action
//     : actionBase { $$ = std::move($1); }
//     | DEFN LID BIND actionBase { $$ = action_ptr(new action_bind(std::move($2), std::move($4))); }
//     ;

// actionBase
//     : OCURLY aOr CCURLY { $$ = action_ptr(new action_exec(std::move($2))); }
//     | RETURN OCURLY aOr CCURLY { $$ = action_ptr(new action_return(std::move($3))); }
//     | OCURLY error CCURLY { REPORT_ERROR("Illegal expr."); }
//     | RETURN OCURLY error CCURLY { REPORT_ERROR("Illegal expr."); }
//     ;

lowercaseParams
    : %empty { $$ = std::vector<std::string>(); }
    | lowercaseParams LID { $$ = std::move($1); $$.push_back(std::move($2)); }
    ;

extendedUID
    : UID { $$ = std::move($1); }
    | INT { $$ = "Int"; }
    | FLOAT { $$ = "Float"; }
    | BOOL { $$ = "Bool"; }
    | STRING { $$ = "String"; }
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
    | app { $$ = std::move($1); }
    ;

app
    : app appBase { $$ = ast_ptr(new ast_app(std::move($1), std::move($2))); }
    | appBase { $$ = std::move($1); }
    ;

appBase
    : FLOATNUMBER { $$ = ast_ptr(new ast_float($1)); }
    | INTEGER { $$ = ast_ptr(new ast_int($1)); }
    // | STRINGINSTANCE { $$ = ast_ptr(new ast_string(std::move($1))); }
    // | TRUE { $$ = ast_ptr(new ast_bool(true)); }
    // | FALSE { $$ = ast_ptr(new ast_bool(false)); }
    | LID { $$ = ast_ptr(new ast_lid(std::move($1))); }
    | extendedUID { $$ = ast_ptr(new ast_uid(std::move($1))); }
    | OPAREN aOr CPAREN { $$ = std::move($2); }
    | case { $$ = std::move($1); }
    // | tuple { $$ = std::move($1); }
    // | list { $$ = std::move($1); }
    // | OSQUARE aOr CSQUARE { $$ = ast_ptr(new ast_index(std::move($2))); }
    // | MINUS %prec NEGATE { $$ = ast_ptr(new ast_uniop(NEGATE)); }
    // | NOT { $$ = ast_ptr(new ast_uniop(NOT)); }
    // | BITNOT { $$ = ast_ptr(new ast_uniop(BITNOT)); }
    | OPAREN error CPAREN { REPORT_ERROR("Illegal expression."); }
    | OSQUARE error CSQUARE { REPORT_ERROR("Illegal expression."); }
    ;

case
    : CASE aOr OF OCURLY branches CCURLY 
        { $$ = ast_ptr(new ast_case(std::move($2), std::move($5))); }
    | CASE aOr OF OCURLY branches error { REPORT_ERROR("Unmatched '{'."); }
    ;

branches
    : branches branch { $$ = std::move($1); $$.push_back(std::move($2)); }
    | branch { $$ = std::vector<branch_ptr>(); $$.push_back(std::move($1));}
    ;

branch
    : pattern ARROW OCURLY aOr CCURLY
        { $$ = branch_ptr(new branch(std::move($1), std::move($4))); }
    | pattern ARROW OCURLY aOr error { REPORT_ERROR("Unmatched '{'."); }
    | pattern ARROW OCURLY error CCURLY { REPORT_ERROR("Illegal expr."); }
    ;

pattern
    : LID { $$ = pattern_ptr(new pattern_var(std::move($1))); }
    | UID lowercaseParams  // ONLY UID HERE
        { $$ = pattern_ptr(new pattern_constr(std::move($1), std::move($2))); }
    ;

data
    : DATA UID lowercaseParams EQUAL OCURLY constructors CCURLY  // ONLY UID HERE.
        { $$ = definition_data_ptr(new definition_data(std::move($2), std::move($3), std::move($6))); }
    ;

constructors
    : constructors COMMA constructor { $$ = std::move($1); $$.push_back(std::move($3)); }
    | constructor
        { $$ = std::vector<constructor_ptr>(); $$.push_back(std::move($1)); }
    ;

constructor
    : UID typeList  // ONLY UID HERE
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

// list
//     : OSQUARE CSQUARE { $$ = ast_ptr(new ast_list(std::vector<ast_ptr>())); }
//     | OSQUARE termlist COMMA CSQUARE { $$ = ast_ptr(new ast_list(std::move($2))); }
//     | OSQUARE termlist COMMA error { REPORT_ERROR("Unmatched '['."); }
//     | OSQUARE termlist CSQUARE error { REPORT_ERROR("Missing ','."); }
//     ;

// tuple
//     : OPAREN termlist COMMA CPAREN { $$ = ast_ptr(new ast_tuple(std::move($2))); }
//     | OPAREN termlist COMMA error { REPORT_ERROR("Unmatched '('."); }
//     | OPAREN termlist CPAREN error { REPORT_ERROR("Missing ','."); }
//     ;

// len(termlist) >= 2 with out a comma, error
// termlistLong 
//     : termlist COMMA aOr { $$ = std::move($1); $$.push_back(std::move($3)); }
//     ;

// termlist
//     : aOr { $$ = std::vector<ast_ptr>(); $$.push_back(std::move($1)); }
//     | termlist COMMA aOr { $$ = std::move($1); $$.push_back(std::move($3)); }
//     ;