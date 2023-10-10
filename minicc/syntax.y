%locations
//%define parse.error detailed
%{
    #include "tree.h"
    #include "lex.yy.c"
    #include <stdio.h>

    //#define YYDEBUG 1
    void yyerror(const char* msg);
    Node* root = NULL;
    int yydebug = 1;
    extern int errorOccur;
%}

%union {
    int type_int;
    float type_float;
    char *type_str;
    Node *type_node;
}


%token <type_node> INT FLOAT ID
%token <type_node> RELOP PLUS MINUS STAR DIV DOT ASSIGNOP 
%token <type_node> AND OR NOT
%token <type_node> COMMA SEMI
%token <type_node> LP RP LB RB LC RC
%token <type_node> TYPE STRUCT RETURN IF ELSE WHILE 



/* declared non-terminals*/

%type <type_node> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier FunDec CompSt VarDec
%type <type_node> OptTag Tag DefList Def DecList Dec  VarList ParamDec
%type <type_node> StmtList Stmt Exp Args 


//set associativity and priority of operators 
%right ASSIGNOP
%left OR AND 
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT


//deal with if-else match problem
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%start Program

%%
Program
    :ExtDefList { $$ = createNode("Program", "", @$.first_line, 1); root = $$; addChild($$, $1);} 
    ;

ExtDefList
    :    { $$ = NULL; }
    | ExtDef ExtDefList { $$ = createNode("ExtDefList", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); }
    ;

ExtDef
    : Specifier FunDec CompSt { $$ = createNode("ExtDef", "", @$.first_line, 3); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Specifier ExtDecList SEMI { $$ = createNode("ExtDef", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Specifier SEMI { $$ = createNode("ExtDef","", @$.first_line, 2); addChild($$, $1); addChild($$, $2); }
    //| error SEMI { yyerror("Missing Specifier"); errorOccur = 1; yyerrok; }
    //| Specifier error { yyerror("Missing \";\""); errorOccur = 1; yyerrok; }
    ;

ExtDecList
    : VarDec { $$ = createNode("ExtDecList","", @$.first_line, 1); addChild($$, $1); }
    | VarDec COMMA ExtDecList { $$ =createNode("ExtDecList","", @$.first_line, 2); addChild($$, $1); }
    | VarDec error ExtDecList { yyerror("Missing \",\""); errorOccur = 1; yyerrok; }
    | VarDec error { yyerror("syntax error"); errorOccur = 1; yyerrok; }
    ;

Specifier
    : StructSpecifier { $$ = createNode("Specifier", "", @$.first_line, 1); addChild($$, $1); }
    | TYPE { $$ = createNode("Specifier", "", @$.first_line, 2); addChild($$, $1); }
    ;

StructSpecifier
    : STRUCT OptTag LC DefList RC { $$ = createNode("StructSpecifier", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5); }
    | STRUCT Tag  { $$ = createNode("StructSpecifier","", @$.first_line, 2); addChild($$, $1); addChild($$, $2); }
    //| STRUCT error LC DefList RC { yyerror("Wrong Identifier"); errorOccur = 1; yyerrok; }
    ;

OptTag
    : ID { $$ = createNode("OptTag", "", @$.first_line, 1); addChild($$, $1); }
    |       { $$ = NULL; }
    ;

Tag
    : ID { $$ = createNode("Tag", "", @$.first_line, 1); addChild($$, $1); }
    ;

VarDec 
    : ID { $$ = createNode("VarDec", "", @$.first_line, 1); addChild($$, $1); }
    | VarDec LB INT RB { $$ = createNode("VarDec", "", @$.first_line, 2); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); } /* array */ 
    | VarDec LB error RB { yyerror("Wrong Index"); errorOccur = 1; yyerrok; }
    | VarDec LB error  { yyerror("Missing \"]\""); errorOccur = 1; yyerrok; }
    //| VarDec LB INT error { yyerror("Missing \"]\""); errorOccur = 1; yyerrok; }
    ;
    
FunDec
    : ID LP RP { $$ = createNode("FunDec", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); addChild($$, $3); } // function without ParamDecs
    | ID LP VarList RP { $$ = createNode("FunDec", "", @$.first_line, 2); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); } // function with ParamDecs
    | ID LP error { yyerror("Missing LB"); errorOccur = 1; yyerrok; }
    | ID LP error RP { yyerror("Wrong arguments"); errorOccur = 1; yyerrok; }
    ;

VarList
    : ParamDec COMMA VarList { $$ = createNode("VarList", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | ParamDec { $$ = createNode("VarList","", @$.first_line, 2); addChild($$, $1); }
    ;

ParamDec
    : Specifier VarDec { $$ = createNode("ParamDec", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); }
    //| error VarDec { yyerror("Missing Specifier"); errorOccur = 1; yyerrok; }
    ;

CompSt
    : LC DefList StmtList RC { $$ = createNode("CompSt", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); }
    //| error DefList StmtList RC { yyerror("Missing LC"); errorOccur = 1; yyerrok; }
    ;

StmtList
    : { $$ = NULL; }
    | Stmt StmtList { $$ = createNode("StmtList", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); }
    ;


Stmt
    : Exp SEMI { $$ = createNode("Stmt", "", @$.first_line, 1);  addChild($$, $1); addChild($$, $2); }
    | CompSt    { $$ = createNode("Stmt", "", @$.first_line, 2);  addChild($$, $1);}
    | RETURN Exp SEMI   { $$ = createNode("Stmt", "", @$.first_line, 3);  addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    //| LC RC { $$ = createNode("Stmt", "", @$.first_line, 4);  addChild($$, $1); addChild($$, $2); }
    //| LC Stmt RC    { $$ = createNode("Stmt", "", @$.first_line, 5);  addChild($$, $1);  addChild($$, $2); addChild($$, $3); } //block
    | TYPE Exp SEMI /* declaration */   { $$ = createNode("Stmt", "", @$.first_line, 6);  addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = createNode("Stmt", "", @$.first_line, 7);  addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5); }
    | IF LP Exp RP Stmt ELSE Stmt { $$ = createNode("Stmt", "", @$.first_line, 8);  addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5); addChild($$, $6); addChild($$, $7);}
    | WHILE LP Exp RP Stmt { $$ = createNode("Stmt", "", @$.first_line, 9);  addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5); }
    //| Exp error { yyerror("Missing \";\""); errorOccur = 1; yyerrok; }
    ;



DefList
    : Def DefList { $$ = createNode("DefList", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); }
    | { $$ = NULL; }
    ;

Def
    : Specifier DecList SEMI { $$ = createNode("Def", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    //| error DecList SEMI { yyerror("Wrong Specifier"); errorOccur = 1; yyerrok; }
    ;

DecList
    : Dec COMMA  DecList { $$ = createNode("DecList", "", @$.first_line, 1); addChild($$, $1);  addChild($$, $2); addChild($$, $3); }
    | Dec { $$ = createNode("DecList", "", @$.first_line, 2); addChild($$, $1); }
    ;

Dec
    : VarDec { $$ = createNode("Dec", "", @$.first_line, 1); addChild($$, $1); }
    | VarDec ASSIGNOP Exp { $$ = createNode("Dec", "", @$.first_line, 2); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    ;

Exp
    : Exp ASSIGNOP Exp { $$ = createNode("Exp", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp RELOP Exp { $$ = createNode("Exp", "", @$.first_line, 2); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp PLUS Exp  { $$ = createNode("Exp", "", @$.first_line, 3); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp MINUS Exp     { $$ = createNode("Exp", "", @$.first_line, 4); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | MINUS Exp { $$ = createNode("Exp", "", @$.first_line, 5); addChild($$, $1); addChild($$, $2); }
    | Exp STAR Exp  { $$ = createNode("Exp", "", @$.first_line, 6); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp DIV Exp   { $$ = createNode("Exp", "", @$.first_line, 7); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp AND Exp   { $$ = createNode("Exp", "", @$.first_line, 8); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp OR Exp    { $$ = createNode("Exp", "", @$.first_line, 9); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | NOT Exp   { $$ = createNode("Exp", "", @$.first_line, 10); addChild($$, $1); addChild($$, $2); }
    | LP Exp RP { $$ = createNode("Exp", "", @$.first_line, 11); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | ID LP RP   { $$ = createNode("Exp", "", @$.first_line, 12); addChild($$, $1); addChild($$, $2); addChild($$, $3); } //function without args
    | ID LP Args RP    { $$ = createNode("Exp", "", @$.first_line, 13); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); } //function with args
    | Exp DOT ID   { $$ = createNode("Exp", "", @$.first_line, 14); addChild($$, $1); addChild($$, $2); addChild($$, $3); } //class
    | Exp LB Exp RB { $$ = createNode("Exp", "", @$.first_line, 15); addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); } //array
    | ID    { $$ = createNode("Exp", "", @$.first_line, 16); addChild($$, $1); }
    | INT   { $$ = createNode("Exp", "", @$.first_line, 17); addChild($$, $1); }
    | FLOAT { $$ = createNode("Exp", "", @$.first_line, 18); addChild($$, $1); }
    //| Exp LB INT error SEMI { yyerror("Missing \"]\""); errorOccur = 1; yyerrok; }
    //| Exp ASSIGNOP error { yyerror("Missing right operand"); errorOccur = 1; yyerrok; }
    ;

Args
    : Exp COMMA Args { $$ = createNode("Args", "", @$.first_line, 1); addChild($$, $1); addChild($$, $2); addChild($$, $3); }
    | Exp { $$ = createNode("Args", "", @$.first_line, 2); addChild($$, $1); }
    ;


%%



void yyerror(const char* msg) {
    fprintf(stderr, "Error type B at Line %d: %s\n", yylineno, msg);
}