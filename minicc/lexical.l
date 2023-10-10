%{
    #include <stdio.h>
    #include "tree.h"
    #include "syntax.tab.h"

    void comment();

    int yycolumn = 1;
    // update the location of lexemes before actions
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng -1; \
        yycolumn += yyleng;
    
    extern int errorOccur;
%}


digit [0-9]
letter [_a-zA-Z]
relop >|<|>=|<=|==|!=
int ([1-9]{digit}*|0)
oct 0[0-7]+
hex 0[xX][0-9a-fA-F]+
float {digit}+\.{digit}+
id {letter}+({digit}|{letter})* 
ws [ \t]+ 

wrongOct 0[0-7]*[89]+{digit}*
wrongHex 0[xX][0-9a-fA-F]*[g-zG-Z]+[0-9a-zA-Z]*

dec1 [1-9]{digit}*\.{digit}+
dec2 [1-9]{digit}+\.

index ({dec1}|{dec2})[eE][+-]?{digit}+

wrongIndex ({dec1}|{dec2})[eE][+-]?

%option yylineno

%%
{ws} { } 

\n { yycolumn = 1; }


"/*" {
    // 处理行间注释 
    comment(); 
}

"//" {
    // 处理单行注释
    char c = input();
    // end-of-line/end-of-file detection
    while(c != '\n' && c != '\0') c = input();
}


"+" { yylval.type_node = createNode("PLUS", yytext, yylineno, 0); return PLUS; }
"," { yylval.type_node = createNode("COMMA", yytext, yylineno, 0); return COMMA; }
"-" { yylval.type_node = createNode("MINUS", yytext, yylineno, 0); return MINUS; }
";" { yylval.type_node = createNode("SEMI", yytext, yylineno, 0); return SEMI; }
"=" { yylval.type_node = createNode("ASSIGNOP", yytext, yylineno, 0); return ASSIGNOP; }
"/" { yylval.type_node = createNode("DIV", yytext, yylineno, 0); return DIV; }
"*" { yylval.type_node = createNode("STAR", yytext, yylineno, 0); return STAR; }
"&&" { yylval.type_node = createNode("AND", yytext, yylineno, 0); return AND; }
"||" { yylval.type_node = createNode("OR", yytext, yylineno, 0); return OR; } 
"." { yylval.type_node = createNode("DOT", yytext, yylineno, 0); return DOT; }
"!" { yylval.type_node = createNode("NOT", yytext, yylineno, 0); return NOT; } 
"int"|"float" { yylval.type_node = createNode("TYPE", yytext, yylineno, 0); return TYPE; }
"(" { yylval.type_node = createNode("LP", yytext, yylineno, 0); return LP; }
")" { yylval.type_node = createNode("RP", yytext, yylineno, 0); return RP; }
"[" { yylval.type_node = createNode("LB", yytext, yylineno, 0); return LB; }
"]" { yylval.type_node = createNode("RB", yytext, yylineno, 0); return RB; }
"{" { yylval.type_node = createNode("LC", yytext, yylineno, 0); return LC; }
"}" { yylval.type_node = createNode("RC", yytext, yylineno, 0); return RC; }
"struct" { yylval.type_node = createNode("STRUCT", yytext, yylineno, 0); return STRUCT; }
"return" { yylval.type_node = createNode("RETURN", yytext, yylineno, 0); return RETURN; }
"if" { yylval.type_node = createNode("IF", yytext, yylineno, 0); return IF; }
"else" { yylval.type_node = createNode("ELSE", yytext, yylineno, 0); return ELSE; }
"while" { yylval.type_node = createNode("WHILE", yytext, yylineno, 0); return WHILE; }
{relop} { yylval.type_node = createNode("RELOP", yytext, yylineno, 0); return RELOP; }


{int} {
        yylval.type_node = createNode("INT", yytext, yylineno, 0);
        return INT; 
}   

{float} { yylval.type_node = createNode("FLOAT", yytext, yylineno, 0); return FLOAT; } 

{id} { yylval.type_node = createNode("ID", yytext, yylineno, 0); return ID; } 

.   { printf("Error type A at Line %d: Unrecognized tokens \'%s\'\n", yylineno, yytext); errorOccur = 1; }

{oct} { yylval.type_node = createNode("OCT", yytext, yylineno, 0); return INT; }

{hex} { yylval.type_node = createNode("HEX", yytext, yylineno, 0); return INT; }

{wrongOct} { printf("Error Type A at Line %d: Illegal octal number \"%s\"\n", yylineno, yytext); errorOccur = 1; return INT; }

{wrongHex} { printf("Error Type A at Line %d: Illegal hexadecimal number \"%s\"\n", yylineno, yytext); errorOccur = 1; return INT; } //{wrongIndex} { printf("Error Type A at line %d: Illegal floating number")}

{index} { yylval.type_node = createNode("INDEX", yytext, yylineno, 0); return FLOAT; }

{wrongIndex} { printf("Error Type A at Line %d: Illegal float point number \"%s\"\n", yylineno, yytext); errorOccur = 1; return FLOAT; }

%%

void comment() {
    // deal with between-line comment /**/
    char prev = 0, c;
    while((c = input()) != 0) {
        if(c == '/' && prev == '*')
            return;
        prev = c;
    }
    printf("Error Type B at line %d: Unterminated comment\n", yylineno);
    errorOccur = 1;
}


int yywrap() {
    return 1;
}
