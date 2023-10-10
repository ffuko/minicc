#include "tree.h"
#include "symbol-table.h"

void Program(Node *root);
void ExtDefList(Node *root);
void ExtDef(Node *root);
void ExtDefGlobal(Node *root);
void ExtDefStruct(Node *root);

void ExtDefFun(Node *root);
void FunDec(Node *root, Type *retType);
Type* CompSt(Node *root, int scope, Type *toRetType);
Type* StmtList(Node *root, int scope, Type *toRetType) ;
Type* Stmt(Node *root, int scope, Type *toRetType);
void VarList(Node *root, Function *fun);
void ParamDec(Node *node, Function *fun);

void ExtDecList(Node *root, Type* type, int scope);
Symbol* VarDec(Node *root, Type* type, int scope);
Type* Specifier(Node *root);
Type* StructSpecifier(Node *root);
void DefListStr(Node *root, FieldList* structure, int scope);
void DefListFun(Node *root, int scope);
void DefFun(Node *root, int scope);
void DecListFun(Node *root, Type *type, int scope);
void DecFun(Node *root, Type *type, int scope);

void DefStr(Node *root, FieldList *structure, int scope);


void DecListStr(Node * root, FieldList *structure, Type *type ,int scope);
void DecStr(Node *root, FieldList *structure, Type *type, int scope);
Type* Exp(Node *root, int scope);
Type* GetTypeFromFun(Node *root);
Type* GetTypeFromVar(Node *root, int scope);

Status CheckFieldOfStr(Type *type, char *name);
void GetArgsOfFun(Node *root, Function *function, int scope);
void CheckArgsEqual(Function *param, Function *arg, int line);


void error(const char *funName);

void PrintType(Type *type);

void PrintFun(Function *fun, char *name);