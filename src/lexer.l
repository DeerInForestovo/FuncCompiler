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
[0-9]+ { return yy::parser::make_INT(atoi(yytext)); }
defn { return yy::parser::make_DEFN(); }
data { return yy::parser::make_DATA(); }
case { return yy::parser::make_CASE(); }
of { return yy::parser::make_OF(); }
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