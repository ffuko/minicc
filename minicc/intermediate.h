#ifndef INTERMEDIATE_H
#define INTERMEDIATE_H

#include "symbol-table.h"
#include "tree.h"

#define MAX_LENGTH 32
#define ERROR "Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n"


typedef struct Operand {
    enum { VAR, CONSTANT, /*ADDRESS, */LABEL, FUN, HEAD, TEMP} kind;
    union {
        int var_no;
        int value;
        int temp_no;
        int label_no;
        char *name; //name for function definition
    }u;
}Operand;

typedef struct InterCode {
    enum { ASSIGN, ADD, SUB, MUL, DIVC, ADDR, POINT, POINT_ASSIGN, FUNC, 
        LABELC, PARAMC, DEC, RETURNC, GOTO, COND, CALL, ARG,
        READ, WRITE
    } kind;
    union {
        struct { Operand *left, *right; } assign;
        struct { Operand *res, *op1, *op2; } binop;
        struct { Operand *res, *op1; } sinop;
        struct { Operand *op; } sin; // function, return, param, label, etc.
        struct { Operand *op; unsigned size; } dec;
        struct { Operand *op1, *op2, *target; char relop[4]; } cond;
    }u;
}InterCode;

typedef struct InterCodes {
    InterCode* code;
    struct InterCodes *prev, *next;
}InterCodes;

void AppendCodes(InterCodes *codes);
void AppendLabel(Operand *label);
Operand* CreateLabel();
void CAConstCodes(Operand *left, int n); //create and append Const Assignment Codes
Operand* CreateConst(int n);
void CAArgCodes(Operand *arg);
void CAGotoCodes(Operand *label); 
void CAMulCodes(Operand *res, Operand *op1, Operand *op2); //TODO: not finished
void CAAddrCodes(Operand *left, Operand *right);
void CAAddCodes(Operand *res, Operand *op1, Operand *op2);
void CAPointCodes(Operand *left, Operand *right);
void CAPointAssignCodes(Operand *left, Operand *right);
Operand *CreateTemp();
Operand *CreateVar();
Operand *CreateFun(char *name);


unsigned GetSizeOfArray(Node *root); // calculate the size of array
unsigned CalculateSize(struct type_ *type);
InterCodes *InitCodes();


void TranslateProgram(Node *root);
void TranslateExtDefList(Node *root);
void TranslateExtDef(Node *root);
void TranslateFunDec(Node *root);
void TranslateCompSt(Node *root);
void TranslateStmtList(Node *root);
void TranslateStmt(Node *root);
void TranslateVarList(Node *root);
void TranslateParamDec(Node *root);
void TranslateVarDecFun(Node *root);
//void TranslateExtDecList(Node *root);
void TranslateVarDec(Node *root);
void TranslateDefList(Node *root);
void TranslateDef(Node *root);
void TranslateDecList(Node *root);
void TranslateDec(Node *root);
void TranslateArgs(Node *root, Operand **arg_list, int index);

void TranslateExp(Node *root, Operand *place);
void TranslateCond(Node *root, Operand *label1, Operand *label2);

void PrintCodes(InterCodes *head);
void PrintOp(Operand *op);
void GenIR(char *filename, InterCodes *head);


void SetEnv(HashTable *table); //添加read/write函数
#endif
