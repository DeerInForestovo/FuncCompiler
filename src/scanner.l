%option noyywrap
%option yylineno

%{
#include <iostream>
#include "ast.hpp"
#include "definition.hpp"
#include "parser.hpp"

#define YY_DECL yy::parser::symbol_type yylex()
extern int yylineno;
int lexer_error_cnt = 0;
void yyerror(const char *s);

%}

%%
\"([^\\\"\n]|\\.)*\" {
     return yy::parser::make_STRINGINSTANCE(std::string(yytext+1, yyleng-2));
}
\"([^\\\"\n]|\\.)*\\? {
    yyerror("Unmatched '\"'.");
    return yy::parser::make_STRINGINSTANCE("");
}
'([^\\'\n]|\\.)' {
    char parsedChar = '\0';
    if (yytext[1] == '\\') {
        switch (yytext[2]) {
            case '0':  parsedChar = '\0'; break;
            case 'n':  parsedChar = '\n'; break;
            case 't':  parsedChar = '\t'; break;
            case 'r':  parsedChar = '\r'; break;
            case '\\': parsedChar = '\\'; break;
            case '\'': parsedChar = '\''; break;
            case '\"': parsedChar = '\"'; break;
            default: yyerror((std::string("Unknown escape sequence: ") + yytext).c_str()); break;
        }
    } else parsedChar = yytext[1];
    return yy::parser::make_CHARINSTANCE(parsedChar);
}
'\\? {
    yyerror("Unmatched '''.");
    return yy::parser::make_CHARINSTANCE('\0');
}
#.* {}
[ \t\r]+ {}
\n {}
\+\+ { return yy::parser::make_CONNECT(); }
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
> { return yy::parser::make_GT(); }
\<= { return yy::parser::make_LEQ(); }
>= { return yy::parser::make_GEQ(); }
== { return yy::parser::make_EQ(); }
!= { return yy::parser::make_NEQ(); }
\<\< { return yy::parser::make_LMOVE(); }
>> { return yy::parser::make_RMOVE(); }
[0-9]*\.[0-9]+([eE][-+]?[0-9]+)? { 
    return yy::parser::make_FLOATNUMBER(atof(yytext)); 
}
[0-9]+[eE][-+]?[0-9]+ { return yy::parser::make_FLOATNUMBER(atof(yytext)); }
[0-9]+ { return yy::parser::make_INTEGER(atoi(yytext)); }
defn { return yy::parser::make_DEFN(); }
data { return yy::parser::make_DATA(); }
case { return yy::parser::make_CASE(); }
of { return yy::parser::make_OF(); }
do { return yy::parser::make_DO(); }
return { return yy::parser::make_RETURN(); }
\{ { return yy::parser::make_OCURLY(); }
\} { return yy::parser::make_CCURLY(); }
\( { return yy::parser::make_OPAREN(); }
\) { return yy::parser::make_CPAREN(); }
\[ { return yy::parser::make_OSQUARE(); }
\] { return yy::parser::make_CSQUARE(); }
,  { return yy::parser::make_COMMA(); }
: { return yy::parser::make_COLON(); }
-> { return yy::parser::make_ARROW(); }
\<- { return yy::parser::make_BIND(); }
= { return yy::parser::make_EQUAL(); }
[a-z][a-zA-Z]* { return yy::parser::make_LID(std::string(yytext)); }
[A-Z][a-zA-Z]* { return yy::parser::make_UID(std::string(yytext)); }

. {yyerror((std::string("Unrecognized token: '") + yytext + "'.").c_str());}

%%

void yyerror(const char *s) {
    std::cerr << "Lexer error at line " << yylineno << ": " << s << std::endl;
    ++lexer_error_cnt;
}