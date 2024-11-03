%option noyywrap

%{
#include <iostream>
#include "ast.hpp"
#include "parser.tab.hh"

#define YY_DECL yy::parser::symbol_type yylex()

%}

%%

[ \n]+ {}
\+ { return yy::parser::make_PLUS(); }
\* { return yy::parser::make_TIMES(); }
- { return yy::parser::make_MINUS(); }
\/ { return yy::parser::make_DIVIDE(); }
% { return yy::parser::make_BMOD(); }
& { return yy::parser::make_BITAND(); }
\| { return yy::parser::make_BITOR(); }
~ { return yy::parser::make_BITNOT(); }
&& { return yy::parser::make_AND(); }
\|\| { return yy::parser::make_OR(); }
! { return yy::parser::make_NOT(); }
\^ { return yy::parser::make_XOR(); }
\< { return yy::parser::make_LT(); }
\> { return yy::parser::make_GT(); }
\<= { return yy::parser::make_LEQ(); }
\>= { return yy::parser::make_GEQ(); }
== { return yy::parser::make_EQ(); }
!= { return yy::parser::make_NEQ(); }
\<\< { return yy::parser::make_LMOVE(); }
\>\> { return yy::parser::make_RMOVE(); }
[0-9]*\.[0-9]+([eE][-+]?[0-9]+)? { return yy::parser::make_FLOATNUMBER(atof(yytext)); }
[0-9]+[eE][-+]?[0-9]+ { return yy::parser::make_FLOATNUMBER(atof(yytext)); }
[0-9]+ { return yy::parser::make_INTEGER(atoi(yytext)); }
\"[^\"]*\" {
    if (yytext[yyleng-2] == '\\') {
        yyless(yyleng-1);
        yymore();
    } else return yy::parser::make_STRINGINSTANCE(std::string(yytext+1, yyleng-2));
}
defn { return yy::parser::make_DEFN(); }
data { return yy::parser::make_DATA(); }
case { return yy::parser::make_CASE(); }
of { return yy::parser::make_OF(); }
True { return yy::parser::make_TRUE(); }
False { return yy::parser::make_FALSE(); }
int { return yy::parser::make_INT(); }
string { return yy::parser::make_STRING(); }
float { return yy::parser::make_FLOAT(); }
bool { return yy::parser::make_BOOL(); }
\{ { return yy::parser::make_OCURLY(); }
\} { return yy::parser::make_CCURLY(); }
\( { return yy::parser::make_OPAREN(); }
\) { return yy::parser::make_CPAREN(); }
\[ { return yy::parser::make_OSQUARE(); }
\] { return yy::parser::make_CSQUARE(); }
,  { return yy::parser::make_COMMA(); }
-> { return yy::parser::make_ARROW(); }
= { return yy::parser::make_EQUAL(); }
[a-z][a-zA-Z_]* { return yy::parser::make_LID(std::string(yytext)); }
[A-Z][a-zA-Z_]* { return yy::parser::make_UID(std::string(yytext)); }

%%