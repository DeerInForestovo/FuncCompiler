%option noyywrap

%{
#include <iostream>
#include "ast.hpp"
#include "definition.hpp"
#include "parser.hpp"

#define YY_DECL yy::parser::symbol_type yylex()
#define UPDATE_LINE_ID do { lst_line_id = cur_line_id; cur_line_id = line_id; } while (0);

int lst_line_id = 1, cur_line_id = 1, line_id = 1;

#define REPORT_ERROR(fmt, args...) do {\
    fprintf(stderr, "An error occured at line %d: syntax error\n", lst_line_id); \
    fprintf(stderr, fmt, ##args); \
    fprintf(stderr, "\n"); \
} while (0);

%}

%%
\"([^\\\"\n]|\\.)*\" {
    UPDATE_LINE_ID return yy::parser::make_STRINGINSTANCE(std::string(yytext+1, yyleng-2));
}
\"([^\\\"\n]|\\.)*\\? {
    UPDATE_LINE_ID REPORT_ERROR("Unmatched \".");
    return yy::parser::make_STRINGINSTANCE("");
}
'([^\\'\n]|\\.)' {
    UPDATE_LINE_ID;
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
            default: REPORT_ERROR("Unknown escape sequence."); break;
        }
    } else parsedChar = yytext[1];
    return yy::parser::make_CHARINSTANCE(parsedChar);
}
'\\? {
    UPDATE_LINE_ID REPORT_ERROR("Unmatched '.");
    return yy::parser::make_CHARINSTANCE('\0');
}
#.* {}
[ \t\r]+ {}
\n { ++line_id; }
\+\+ { UPDATE_LINE_ID return yy::parser::make_CONNECT(); }
\+ { UPDATE_LINE_ID return yy::parser::make_PLUS(); }
\* { UPDATE_LINE_ID return yy::parser::make_TIMES(); }
- { UPDATE_LINE_ID return yy::parser::make_MINUS(); }
\/ { UPDATE_LINE_ID return yy::parser::make_DIVIDE(); }
% { UPDATE_LINE_ID return yy::parser::make_BMOD(); }
& { UPDATE_LINE_ID return yy::parser::make_BITAND(); }
\| { UPDATE_LINE_ID return yy::parser::make_BITOR(); }
~ { UPDATE_LINE_ID return yy::parser::make_BITNOT(); }
&& { UPDATE_LINE_ID return yy::parser::make_AND(); }
\|\| { UPDATE_LINE_ID return yy::parser::make_OR(); }
! { UPDATE_LINE_ID return yy::parser::make_NOT(); }
\^ { UPDATE_LINE_ID return yy::parser::make_XOR(); }
\< { UPDATE_LINE_ID return yy::parser::make_LT(); }
\> { UPDATE_LINE_ID return yy::parser::make_GT(); }
\<= { UPDATE_LINE_ID return yy::parser::make_LEQ(); }
\>= { UPDATE_LINE_ID return yy::parser::make_GEQ(); }
== { UPDATE_LINE_ID return yy::parser::make_EQ(); }
!= { UPDATE_LINE_ID return yy::parser::make_NEQ(); }
\<\< { UPDATE_LINE_ID return yy::parser::make_LMOVE(); }
\>\> { UPDATE_LINE_ID return yy::parser::make_RMOVE(); }
[0-9]*\.[0-9]+([eE][-+]?[0-9]+)? { 
    UPDATE_LINE_ID return yy::parser::make_FLOATNUMBER(atof(yytext)); 
}
[0-9]+[eE][-+]?[0-9]+ { UPDATE_LINE_ID return yy::parser::make_FLOATNUMBER(atof(yytext)); }
[0-9]+ { UPDATE_LINE_ID return yy::parser::make_INTEGER(atoi(yytext)); }
defn { UPDATE_LINE_ID return yy::parser::make_DEFN(); }
data { UPDATE_LINE_ID return yy::parser::make_DATA(); }
case { UPDATE_LINE_ID return yy::parser::make_CASE(); }
of { UPDATE_LINE_ID return yy::parser::make_OF(); }
do { UPDATE_LINE_ID return yy::parser::make_DO(); }
return { UPDATE_LINE_ID return yy::parser::make_RETURN(); }
\{ { UPDATE_LINE_ID return yy::parser::make_OCURLY(); }
\} { UPDATE_LINE_ID return yy::parser::make_CCURLY(); }
\( { UPDATE_LINE_ID return yy::parser::make_OPAREN(); }
\) { UPDATE_LINE_ID return yy::parser::make_CPAREN(); }
\[ { UPDATE_LINE_ID return yy::parser::make_OSQUARE(); }
\] { UPDATE_LINE_ID return yy::parser::make_CSQUARE(); }
,  { UPDATE_LINE_ID return yy::parser::make_COMMA(); }
-> { UPDATE_LINE_ID return yy::parser::make_ARROW(); }
\<- { UPDATE_LINE_ID return yy::parser::make_BIND(); }
= { UPDATE_LINE_ID return yy::parser::make_EQUAL(); }
[a-z][a-zA-Z_]* { UPDATE_LINE_ID return yy::parser::make_LID(std::string(yytext)); }
[A-Z][a-zA-Z_]* { UPDATE_LINE_ID return yy::parser::make_UID(std::string(yytext)); }

. { UPDATE_LINE_ID REPORT_ERROR("Unexpected character '%s'", yytext); }

%%