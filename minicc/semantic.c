#include "semantic.h"
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "symbol-table.h"
#include <stdio.h>

extern HashTable *sym_table;


void Program(Node *root) {
    //printf("%s\n", __FUNCTION__);
    if(root)
        ExtDefList(root->child);
    return;
}


void ExtDefList(Node *root) {
    //printf("%s\n", __FUNCTION__);
    if(!root)
        return;
    Node *child = root->child; //first ExtDef
    ExtDef(child);
    ExtDefList(child->next);
    return;
}

void ExtDef(Node *root) {
    //printf("%s\n", __FUNCTION__);
    switch(root->opt) {
        case 1: //ExtDef->Specifier ExtDecList SEMI, 全局变量定义
            ExtDefGlobal(root);
            break;
        case 2: //ExtDef->Specifier SEMI, 结构体定义
            ExtDefStruct(root);
            break;
        case 3: //ExtDef->Specifier FunDec CompSt, 函数定义
            ExtDefFun(root);
            break;
        default:
            error(__FUNCTION__);
    }
    return;
} 

void ExtDefGlobal(Node *root) { //ExtDef->Specifier ExtDecList SEMI
    //printf("%s\n", __FUNCTION__);
    Type* type = Specifier(root->child); // get type from Specifier 
    ExtDecList(root->child->next, type, GLOBAL);
    return;
}

void ExtDefStruct(Node *root) {  // ExtDef->Specifier SEMI
    //printf("%s\n", __FUNCTION__);
    Type* type = Specifier(root->child);
    if(!type)
        error(__FUNCTION__);
    return;
}

void ExtDefFun(Node *root) { //ExtDef->Specifier FunDec CompSt
    //printf("%s\n", __FUNCTION__);
    Type* toRetType = Specifier(root->child);
    FunDec(root->child->next, toRetType);
    Type* ret_type = CompSt(root->child->next->next, GLOBAL+2, toRetType);
    return;
}

void FunDec(Node *root, Type *retType) {
    //printf("%s\n", __FUNCTION__);
    Function* fun = (Function*)malloc(sizeof(Function)); 
    //printf("After malloc a Function:\n");
    fun->ret_type = retType; 
    fun->op = NULL;
    Symbol *newSym;
    switch(root->opt) {
        case 1: //FunDec->ID LP RP
            fun->argc = 0;
            fun->args = NULL;
            newSym = CreateFunSymbol(root->child->value, fun);
            //printf("After Creating a Funtion Symbol\n");
            if(!InsertHash(sym_table, newSym))
                fprintf(stderr, "Error type 4 at line %d: Redefined function \'%s\'.\n", root->line, newSym->name);
            //PrintTable(sym_table);
            break;
        case 2: //FunDec->ID LP VarList RP
            fun->argc = 0;
            fun->args = NULL;
            VarList(root->child->next->next, fun);
            newSym = CreateFunSymbol(root->child->value, fun);
            if(!InsertHash(sym_table, newSym))
                fprintf(stderr, "Error type 4 at line %d: Redefined function \'%s\'.\n", root->line, newSym->name);
            //PrintTable(sym_table);
            break;
    }
    //PrintFun(fun, root->child->value);
}

Type* CompSt(Node *root, int scope, Type *toRetType) { // CompSt-> LC DefList StmtList RC
    //printf("%s\n", __FUNCTION__);
    Type *ret_type = NULL;
    if(strcmp(root->child->next->name, "RC") == 0)
        return ret_type;
        //printf("LC and RC!\n");
    if(strcmp(root->child->next->name, "DefList") == 0) {
        DefListFun(root->child->next, scope);
        if(root->child->next->next && strcmp(root->child->next->next->name, "StmtList") == 0)
            ret_type = StmtList(root->child->next->next, scope, toRetType);
          
    }
    else if (strcmp(root->child->next->name, "StmtList") == 0) {
        ret_type = StmtList(root->child->next, scope, toRetType);
    }
    return ret_type;
}


Type* StmtList(Node *root, int scope, Type *toRetType) {
    if(root) {//StmtList->Stmt StmtList
        //printf("%s\n", __FUNCTION__);
        Type *ret_type1 = Stmt(root->child, scope, toRetType);
        Type *ret_type2 = StmtList(root->child->next, scope, toRetType);
        return ret_type1 ? ret_type1 : ret_type2;
    }
    return NULL;
}

Type* Stmt(Node *root, int scope, Type *toRetType) {
    //printf("%s, opt: %d\n", __FUNCTION__, root->opt);
    Type *ret_type = NULL;
    switch(root->opt) {
        case 1: //Stmt->Exp SEMI;
            Exp(root->child, scope);
            break;
        case 2: //Stmt->CompSt
            ret_type = CompSt(root->child, scope+1, toRetType);
            return ret_type;
            break;
        case 3: //Stmt->RETURN Exp SEMI
            ret_type = Exp(root->child->next, scope);
            if(ret_type == NULL)
                return NULL;
            if(ret_type->kind != toRetType->kind)  
                fprintf(stderr, "Error type 8 at line %d: return types don't match!\n", root->line);
            return ret_type;
            break;
        /* case 4: //Stmt->LC RC
            break;
        case 5: //Stmt->LC Stmt RC
            if(ret_type = Stmt(root->child->next, scope+1, toRetType))
                return ret_type;
            break; */
        case 6: { //Stmt->TYPE Exp SEMI 
            Type *type = (Type*)malloc(sizeof(Type));
            if(strcmp(root->child->value, "INT") == 0) {
                type->kind = BASIC_INT;
                type->u.int_value = 0;
            }
            else if(strcmp(root->child->value, "FLOAT") == 0) {
                type->kind = BASIC_FLOAT;
                type->u.float_value = 0;
            }
            Type *exp_type = Exp(root->child->next, scope);
            if(!check_type(type, exp_type))
                fprintf(stderr, "Error type 5 at line %d: Type mismatch for assignment.\n", root->line);
            break;
            }
        case 7: {//Stmt->IF LP Exp RP Stmt %prec LOWER_THAN_ELSE 
            Node *exp_node1 = root->child->next->next;
            Exp(exp_node1, scope+1);
            if(ret_type = Stmt(exp_node1->next->next, scope+1, toRetType))
                return ret_type;
            break;
            }
        case 8: { //Stmt->IF LP Exp RP Stmt ELSE Stmt
            Node *exp_node2 = root->child->next->next;
            Exp(exp_node2, scope+1);
            Type *ret_type1 = Stmt(exp_node2->next->next, scope+1, toRetType);
            Type *ret_type2 = Stmt(exp_node2->next->next->next->next, scope+1, toRetType);
            
            return ret_type1 ? ret_type1 : ret_type2;
            break;
            }
        case 9: //Stmt->WHILE LP Exp RP Stmt
            Exp(root->child->next->next, scope+1);
            if(ret_type = Stmt(root->child->next->next->next->next, scope+1, toRetType))
                return ret_type;
            break;
    }
    return NULL;
}

void VarList(Node *root, Function *fun) {
    //printf("%s\n", __FUNCTION__);
    switch(root->opt) {
        case 1: //VarList->ParamDec COMMA VarList
            ParamDec(root->child, fun);
            VarList(root->child->next->next, fun);
            break;
        case 2: //VarList->ParamDec
            ParamDec(root->child, fun);
            break;
    }
    return;
}

void ParamDec(Node *root, Function *fun) { // ParamDec->Specifier VarDec
    //printf("%s\n", __FUNCTION__);
    Type *type = Specifier(root->child);

    fun->argc++;
    FieldList* cur = fun->args;
    
    //get new param
    Symbol *sym = VarDec(root->child->next, type, LOCAL);
    if(sym == NULL)
        return;
    FieldList *newField = (FieldList*)malloc(sizeof(FieldList));
    newField->type = sym->u.variable->type;
    newField->name = (char*)malloc(NAME_SIZE);
    newField->tail = NULL;
    strcpy(newField->name, sym->name);

    if(cur == NULL) {
        fun->args = newField;
    }
    else {
        while(cur->tail != NULL)
            cur = cur->tail;
            cur->tail = newField;
    }
}



void ExtDecList(Node *root, Type* type, int scope) { 
    //printf("%s\n", __FUNCTION__);
    Node *child = root->child;
    switch(root->opt) {
        case 1: // ExtDecList->VarDec
            VarDec(child, type, scope);
            break;
        case 2:
            VarDec(child, type, scope); // ExtDecList->VarDec ExtDecList
            ExtDecList(child->next->next, type, scope);
            break;
    }
    return;
}

Symbol* VarDec(Node *root, Type* type, int scope) {

    //printf("%s at line %d\n", __FUNCTION__, root->line);
    Symbol *newSym;
    switch(root->opt) {
        case 1: { //VarDec->ID
            Variable *newVar = (Variable*)malloc(sizeof(Variable));
            newVar->type = type;
            newVar->scope = scope;
            newVar->op = NULL;
            newSym = CreateVarSymbol(root->child->value, newVar);
            if(scope != PARAM && !InsertHash(sym_table, newSym)) {
                fprintf(stderr, "Error type 3 at line %d: Variable \'%s\' redefined.\n", root->line, newSym->name);
                return NULL;
            }
            else if(scope == PARAM && !InsertHash(sym_table, newSym)) {
                fprintf(stderr, "Error type 15 at line %d: Redefined field \'%s\'\n", root->line, newSym->name);
                return NULL;
            }
            //PrintTable(sym_table);
            break;
        }
        case 2: { //VarDec-> VarDec LB INT RB
            Type* newType = (Type*)malloc(sizeof(Type));
            newType->kind = ARRAY;
            newType->u.array.elem = type;
            newType->u.array.size = atoi(root->child->next->next->value);
            newSym = VarDec(root->child, newType, scope);
            break;
        }
    }
    return newSym;
}




Type* Specifier(Node *root) { // get type from Specifier 
    //printf("%s\n", __FUNCTION__);
    Type* type = (Type*)malloc(sizeof(Type));
    switch(root->opt) { // get kind of type
        case 2: { //Specifier->TYPE, basic type
            Node *child = root->child;
            if(strcmp(child->value, "int") == 0) {
                type->kind = BASIC_INT;
                type->u.int_value = 0;
            }
            else if(strcmp(child->value, "float") == 0) {
                type->kind = BASIC_FLOAT;
                type->u.float_value = 0;
            }
            break;
        }
        case 1: //Specifier->StructSpecifier
            type = StructSpecifier(root->child);
            //if(type != NULL)
                //printf("Find Struct Type\n");
            break;
        default: 
            error(__FUNCTION__);
            return NULL;
    }
    return type;
}

Type* StructSpecifier(Node *root) {
    //printf("%s, %d\n", __FUNCTION__, root->opt);
    Type *type = (Type*)malloc(sizeof(Type));
    switch(root->opt) {
        case 1: { //StructSpecifier->STRUCT OptTag LC DefList RC
            if(strcmp(root->child->next->name, "LC") != 0) {//OptTag->ID 
                type->kind = STRUCTURE;
                type->u.structure = (FieldList*)malloc(sizeof(FieldList));
                type->u.structure->name = ""; //链表的标志头
                type->u.structure->type = NULL;
                type->u.structure->tail = NULL;
                DefListStr(root->child->next->next->next, type->u.structure, GLOBAL+1);
                Variable* newVar = (Variable*)malloc(sizeof(Variable));
                newVar->op = NULL;
                newVar->type = type;
                newVar->scope = GLOBAL;
                Symbol* newSym = CreateVarSymbol(root->child->next->child->value, newVar);
                if(!InsertHash(sym_table, newSym))
                    fprintf(stderr, "Error type 16 at line %d: Duplicated name \'%s\'.\n", root->line, newSym->name);
                //PrintTable(sym_table);
            }
            else {
                //printf("?\n");
                type->kind = STRUCTURE;
                type->u.structure = (FieldList*)malloc(sizeof(FieldList));
                type->u.structure->name = ""; //链表的标志头
                type->u.structure->type = NULL;
                type->u.structure->tail = NULL;
                DefListStr(root->child->next->next, type->u.structure, GLOBAL+1);
            }
            break;
        }
        case 2: //StructSpecifier->STRUCT TAG, 结构体变量声明; Tag->ID
            type =  GetTypeFromVar(root->child->next->child, GLOBAL);
            if(type == NULL)
                fprintf(stderr, "Error type 17 at line %d: Undefined structure \'%s\'.\n", root->line, root->child->next->child->value);
            break;
    }
    return type;
}


void DefListStr(Node *root, FieldList* structure, int scope) {//local variable in a structure
    //printf("%s\n", __FUNCTION__);
    if(root != NULL) {
        DefStr(root->child, structure, scope);
        DefListStr(root->child->next, structure, scope);
    }
    return;
}



void DefListFun(Node *root, int scope) { // local variable in a function; DefList->Def DefList
    //printf("%s\n", __FUNCTION__);
    if(root) {
        DefFun(root->child, scope);
        DefListFun(root->child->next, scope);
    }
    return;
}

void DefFun(Node *root, int scope) { //DefFun->Specifier DecList SEMI
    //printf("%s\n", __FUNCTION__);
    if(root == NULL)
        printf("DefFun is NULL!\n");
    Type *type = Specifier(root->child);
    DecListFun(root->child->next, type, scope);
    return;
}

void DecListFun(Node *root, Type *type, int scope) { 
    //printf("%s\n", __FUNCTION__);
    switch(root->opt) {
        case 1: //DecList->Dec COMMA DecList
            DecFun(root->child, type, scope);
            DecListFun(root->child->next->next, type, scope);
            break;
        case 2: //DecList->Dec
            DecFun(root->child, type, scope);
            break;
    }
}

void DecFun(Node *root, Type *type, int scope) {
    //printf("%s\n", __FUNCTION__);
    Symbol *newSym;
    switch(root->opt) {
        case 1: //Dec->VarDec
            newSym = VarDec(root->child, type, scope);
            //PrintTable(sym_table);
            break;
        case 2: { //Dec->VarDec ASSIGNOP Exp
            newSym = VarDec(root->child, type, scope);
            //PrintTable(sym_table);
            Type *expType = Exp(root->child->next->next, scope); 
            if(expType && expType->kind != type->kind)
                fprintf(stderr, "Error type 5 at line %d: Type mismatch for assignment.\n", root->line);
            break;
        }
    }
}


void DefStr(Node *root, FieldList *structure, int scope) { //Def in struct;Def->Specifier DecList SEMI
    //printf("%s\n", __FUNCTION__);
    Type *type = Specifier(root->child);
    DecListStr(root->child->next, structure, type, scope);
    return;
}

void DecListStr(Node * root, FieldList *structure, Type *type ,int scope) { // DecList in def of struct 
    //printf("%s\n", __FUNCTION__);
    switch(root->opt) {
        case 1: //DecList->Dec COMMA DecList
            DecStr(root->child, structure, type, scope);
            DecListStr(root->child->next->next, structure, type, scope);
            break;
        case 2: //DecList->Dec
            DecStr(root->child, structure, type, scope);
            break;
        default:
            error(__FUNCTION__);
    }
    return;
}

void DecStr(Node *root, FieldList *structure, Type *type, int scope) {
    //printf("%s\n", __FUNCTION__);
    Symbol *newSym;
    FieldList *cur;
    FieldList *newField;
    switch(root->opt) {
        case 1: // Dec->VarDec
            newSym = VarDec(root->child, type, scope);
            //PrintTable(sym_table);
            
            if(newSym == NULL)
                return;
            newField = (FieldList*)malloc(sizeof(FieldList));
            newField->type = type;
            newField->tail = NULL;
            newField->name = (char*)malloc(NAME_SIZE);
            strcpy(newField->name, newSym->name);

            if(structure == NULL)
                error(__FUNCTION__);
                //structure = newField;
            else{
                cur = structure;
                while(cur->tail != NULL) 
                    cur = cur->tail;
                cur->tail = newField;
            } 
            break;
        case 2: // Dec->VarDec ASSIGNOP Exp
            fprintf(stderr, "Error type 15 at line %d: Cannot initialize field when define struct.\n", root->line);
        /*
            newSym = VarDec(root->child, type, scope);
            if(!InsertHash(sym_table, newSym)) 
                fprintf(stderr, "Error type 3 at line %d: Redefined variable \'%s\'.\n", root->line, newSym->name);
            //PrintTable(sym_table);
        
            newField = (FieldList*)malloc(sizeof(FieldList));
            newField->type = type;
            newField->tail = NULL;
            newField->name = (char*)malloc(NAME_SIZE);
            strcpy(newField->name, newSym->name);
            
            if(structure == NULL)
                //error(__FUNCTION__);
                structure = newField;
            else{
                cur = structure;
                while(cur->tail != NULL) 
                    cur = cur->tail;
                cur->tail = newField;
            } 
            
            Type *expType = Exp(root->child->next->next, scope);
            if(expType->kind != type->kind)
                fprintf(stderr, "Error type 5 at line %d: Type mismatched for assignment.\n", root->line);
        */
            break;
    }
    return;
}

Type* Exp(Node *root, int scope) {
    //printf("%s, opt: %d, scope: %d\n", __FUNCTION__, root->opt, scope);
    Type *type;
    Type *exp_type1, *exp_type2;
    switch(root->opt) {
        case 1: //Exp->Exp ASSIGNOP Exp
            exp_type1 = Exp(root->child, scope);
            exp_type2 = Exp(root->child->next->next, scope);
            if(root->child->opt != 16 && root->child->opt != 15 && root->child->opt != 14) { // Exp ASSIGNOP Exp 
                fprintf(stderr, "Error type 6 at line %d:  The left-hand side of an assignment must be a variable.\n", root->line);
            }
            if(exp_type1 && exp_type2 && check_type(exp_type1, exp_type2))
                return exp_type1;
            else if (exp_type1 && exp_type2) {
                fprintf(stderr, "Error type 5 at line %d: Type mismatched for assignment.\n", root->line);
                return NULL;
            }
            break;
        case 2: //Exp->Exp RELOP Exp
        case 8: //Exp->Exp AND Exp
        case 9: //Exp->Exp OR Exp
            exp_type1 = Exp(root->child, scope);
            exp_type2 = Exp(root->child->next->next, scope);
            if(exp_type1 && exp_type2 && check_type(exp_type1, exp_type2)){
                Type *bool_type = (Type*)malloc(sizeof(Type));
                bool_type->kind = BASIC_INT; //布尔表达式的返回类型定为INT
                bool_type->u.int_value = 0;
                return bool_type;
            }
            /*
            else if (exp_type1 && exp_type2) {
                fprintf(stderr, "Error type 7 at line %d: Type mismatched for operands.\n", root->line);
                return NULL;
            }
            */ //TODO: check type if boolean expression
            break;
        case 3: //Exp->Exp PLUS Exp 
        case 4: //Exp->Exp MINUS Exp
        case 6: //Exp->Exp STAR Exp
        case 7: //Exp->Exp DIV Exp
            exp_type1 = Exp(root->child, scope);
            exp_type2 = Exp(root->child->next->next, scope);
            if(exp_type1 && exp_type2 && check_type(exp_type1, exp_type2))
                return exp_type1;
            else if (exp_type1 && exp_type2) {
                fprintf(stderr, "Error type 7 at line %d: Type mismatched for operands.\n", root->line);
                return NULL;
            }
            break;
        case 5: //Exp->MINUS Exp
        case 10:    //Exp->NOT Exp
            return Exp(root->child->next, scope); 
        case 11:    //Exp->LP Exp RP
            return Exp(root->child->next, scope);
        case 12:    //Exp->ID LP RP
        case 13:   { //Exp->ID LP Args RP
            type = GetTypeFromFun(root->child);
            Symbol *sym = FindFunSymbolByName(sym_table, root->child->value);
            if(sym == NULL) {
                Symbol *symIfVar = FindVarSymbolByName(sym_table, root->child->value, scope);
                if(symIfVar){
                    fprintf(stderr, "Error type 11 at line %d: \'%s\' is not a function.\n", root->line, symIfVar->name);
                    return NULL;
                }
                fprintf(stderr, "Error type 2 at line %d: Undefined function \"%s\".\n", root->line, root->child->value);
                return NULL;
            }
            
            Function* fun = (Function*)malloc(sizeof(Function)); 
            fun->op = NULL;
            fun->argc = 0;
            fun->args = NULL;
            fun->ret_type = NULL;
            GetArgsOfFun(root->child->next->next, fun, scope);
            //PrintFun(sym->u.function ,sym->name);
            //PrintFun(fun, "To check");
            CheckArgsEqual(sym->u.function, fun, root->line);
            return type;
        }
        case 14:   { //Exp->Exp DOT ID
            Type *struct_type = Exp(root->child, scope);
            if(!struct_type)
                return NULL;
            if(struct_type->kind != STRUCTURE)
                fprintf(stderr, "Error type 13 at line %d: Illegal use of \'.\'.\n", root->line);
            else if(!CheckFieldOfStr(struct_type, root->child->next->next->value)) {
                fprintf(stderr, "Error type 14 at line %d: Non-existent field \'%s\'\n", root->line, root->child->next->next->value);
                return NULL;
            }
            return GetTypeFromVar(root->child->next->next, scope);
        }
        case 15:  {  //Exp->Exp LB Exp RB
            Type *array_type = Exp(root->child, scope);
            if(array_type && array_type->kind != ARRAY) {
                fprintf(stderr, "Error type 10 at line %d: it is not an array.\n", root->line);
                return NULL;
            }
            Type *index_type = Exp(root->child->next->next, scope);
            if(index_type->kind != BASIC_INT) {
                fprintf(stderr, "Error type 12 at line %d: the index is not an integer.\n", root->line);
                return NULL;
            }
        
            return array_type? array_type->u.array.elem: NULL;
        }
        case 16:    //Exp->ID
            type = GetTypeFromVar(root->child, scope);
            if(type == NULL)
                fprintf(stderr,"Error type 1 at line %d: Undefined variable \'%s\'\n", root->line, root->child->value);
            return type;
        case 17:    //Exp->INT
            type = (Type*)malloc(sizeof(Type));
            type->kind = BASIC_INT;
            type->u.int_value = 0;
            return type;
        case 18:    //Exp->FLOAT
            type = (Type*)malloc(sizeof(Type));
            type->kind = BASIC_FLOAT;
            type->u.float_value = 0;
            return type;
    }
    return type;
}

Type* GetTypeFromFun(Node *root) { //ID
    //printf("%s\n", __FUNCTION__);
    Symbol *sym = FindFunSymbolByName(sym_table, root->value);
    if(sym == NULL)
        return NULL;
    return sym->u.function->ret_type;
}

Type* GetTypeFromVar(Node *root, int scope) { //ID
    //printf("%s\n", __FUNCTION__);
    Symbol *sym = FindVarSymbolByName(sym_table, root->value, scope);
    if(sym == NULL)
        return NULL;
    return sym->u.variable->type;
}

Status CheckFieldOfStr(Type *type, char *name) { //TODO: 考虑作用域
    //printf("%s\n", __FUNCTION__);
    FieldList *cur = type->u.structure->tail;
    while(cur != NULL) {
        if(strcmp(cur->name, name) == 0)
            return STATE_SUCCESS;
        //if(cur->type->kind == STRUCTURE) //嵌套的结构体定义
            //return CheckFieldOfStr(cur->type, name);
        cur = cur->tail;
    }
    return STATE_FAILURE;
}

void GetArgsOfFun(Node *root, Function *fun, int scope) { //root:Args
    //printf("%s\n", __FUNCTION__);
    if(strcmp(root->name, "RP") == 0) //Exp->LP ID RP
        return;
    FieldList* newField = (FieldList*)malloc(sizeof(FieldList));
    switch(root->opt) {
        case 2: { //Args->Exp
            fun->argc++;
            newField->type = Exp(root->child, scope);
            newField->name = (char*)malloc(NAME_SIZE);
            newField->tail = NULL;
            //实参的占位符
            strcpy(newField->name, "");
            if(fun->args == NULL)
                fun->args = newField;
            else {
                FieldList *cur = fun->args;
                while(cur->tail)
                    cur = cur->tail;
                cur->tail = newField;
            
            }  
            return;
        }
        case 1: { //Args->Exp COMMA Args
            fun->argc++;
            newField->type = Exp(root->child, scope);
            newField->name = (char*)malloc(NAME_SIZE);
            newField->tail = NULL;
            //实参的占位符
            strcpy(newField->name, "");

            if(fun->args == NULL)
                fun->args = newField;
            else {
                FieldList *cur = fun->args;
                while(cur->tail)
                    cur = cur->tail;
                cur->tail = newField;  
            }  
            GetArgsOfFun(root->child->next->next, fun, scope);
            return;
        }
    }
}

void CheckArgsEqual(Function *param, Function *arg, int line) {
    //printf("%s\n", __FUNCTION__);
    if(param->argc != arg->argc)
        fprintf(stderr, "Error type 9 at line %d: Unexpected number of arguments.\n", line);
    else {
        FieldList *param_arg = param->args, *arg_arg = arg->args;

        while(param_arg && arg_arg) {
            //PrintFun(param, "to-check params");
            //PrintFun(arg, "to-check arguments");
            if(!check_type(param_arg->type, arg_arg->type)) {
                fprintf(stderr, "Error type 9 at line %d: Wrong type of function arguments.\n", line);
                return;
            }
            param_arg = param_arg->tail;
            arg_arg = arg_arg->tail;
        }
    }
    return;
}



void error(const char *funName) {
    //printf("%s\n", __FUNCTION__);
    printf("Error occurs at %s()\n", funName);
    return;
}

void PrintType(Type *type) {
    if(type != NULL)
        printf("Type kind: %d\n", type->kind);
}

void PrintFun(Function *fun, char *name) {
    if(fun == NULL) {
        printf("Function %s not exists.\n", name);
        return;
    }
    int count = 0;
    printf("Function %s includes  %d arguments: \n", name, fun->argc);
    FieldList *cur = fun->args;
    while(cur) {
        printf("arg %d: ", count);
        //PrintType(cur->type);
        cur = cur->tail;
        count++;
    }

}