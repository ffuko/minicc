#include "intermediate.h"
#include "symbol-table.h"
#include <string.h>
#include <stdio.h>

extern HashTable *sym_table;
extern InterCodes *head;
extern int var_n;
extern int temp_n;
extern int label_n;


void TranslateProgram(Node *root) {
    //Program -> ExtDefList;
    if(root)
        TranslateExtDefList(root->child);
    return;
}

void TranslateExtDefList(Node *root) {
    //ExtDefList -> NULL | ExtDef ExtDefList;
    if(root) {
        TranslateExtDef(root->child);
        TranslateExtDefList(root->child->next);
    }
    return;
}

void TranslateExtDef(Node *root) {
    //不考虑结构体与全局变量
    if(!root) 
        return;
    switch(root->opt) {
        case 3: { // ExtDef -> Specifier FunDec Compst
            TranslateFunDec(root->child->next);
            TranslateCompSt(root->child->next->next);
            break;
        }
        default:
            break;
    }
    return;
}


void TranslateFunDec(Node *root) {
    if(!root)
        return;
    switch(root->opt) {
        case 1:  { //FunDec -> ID LP RP
            //operand for function name
            Operand *new_op = (Operand*)malloc(sizeof(Operand));
            new_op->kind = FUN;
            new_op->u.name = (char*)malloc(sizeof(char) * MAX_LENGTH);
            strcpy(new_op->u.name, root->child->value);
            Symbol *sym = FindFunSymbolByName(sym_table, root->child->value);
            sym->u.function->op = new_op;

            //generate FUNCTION
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            new_code->kind = FUNC;
            new_code->u.sin.op = new_op;

            new_codes->code = new_code;
            new_codes->prev = NULL;
            new_codes->next = NULL;

            AppendCodes(new_codes);
            return;
        }
        case 2: { //FunDec -> ID LP VarList RP
            Operand *new_op = (Operand*)malloc(sizeof(Operand));
            new_op->kind = FUN;
            new_op->u.name = (char*)malloc(sizeof(char) * MAX_LENGTH);
            strcpy(new_op->u.name, root->child->value);
            Symbol *sym = FindFunSymbolByName(sym_table, root->child->value);
            sym->u.function->op = new_op;

            //generate FUNCTION
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            new_code->kind = FUNC;
            new_code->u.sin.op = new_op;

            new_codes->code = new_code;
            new_codes->prev = NULL;
            new_codes->next = NULL;

            AppendCodes(new_codes);

            //generate varlist;
            TranslateVarList(root->child->next->next);
            return;
        }
        default: 
            break;
    }
}

void TranslateVarList(Node *root) {
    if(!root)
        return;
    switch(root->opt) {
        case 1:  // VarList -> ParamDec COMMA VarList
            TranslateParamDec(root->child);
            TranslateVarList(root->child->next->next);
            return;
        case 2: //VarList -> ParamDec
            TranslateParamDec(root->child);
            return;
        default:
            break;
    }
}

void TranslateParamDec(Node *root) {
    if(root) { // ParamDec -> Specifier VarDec;
        TranslateVarDecFun(root->child->next);
    }
}

void TranslateVarDecFun(Node *root) {
    if(root) {
        switch(root->opt) {
            case 1: //VarDec -> ID
            case 2: { //VarDec -> ID LB INT RB
                Operand *new_op = (Operand*)malloc(sizeof(Operand));
                new_op->kind = VAR;
                new_op->u.var_no = ++var_n;
                Symbol *sym = FindVarSymbolByName(sym_table, root->child->value, 100);
                sym->u.variable->op = new_op;

                InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
                InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
                new_code->kind = PARAMC;
                new_code->u.sin.op = new_op;
                
                new_codes->code = new_code;
                new_codes->prev = NULL;
                new_codes->next = NULL;
                
                AppendCodes(new_codes);
                return;
            }
        }
    }
}


void TranslateCompSt(Node *root) {
    //CompSt -> LC DefList StmtList RC
    if(!root)
        return;
    if(strcmp(root->child->next->next->name, "RC") !=0 ) {
        TranslateDefList(root->child->next);
        TranslateStmtList(root->child->next->next);
    }
    else 
        TranslateStmtList(root->child->next);
    return;
}

void TranslateDefList(Node *root) {
    if(root) { //DefList -> Def DefList;
        TranslateDef(root->child);
        TranslateDefList(root->child->next);
    }
}

void TranslateDef(Node *root) {
    if(root) { // Def -> Specifier DecList SEMI
        TranslateDecList(root->child->next);
    }
}
void TranslateDecList(Node *root) {
    if(root) { // DecList -> Dec COMMA DecList | Dec
        switch(root->opt) {
        case 1:
            TranslateDec(root->child);
            TranslateDecList(root->child->next->next);
            break;
        case 2:
            TranslateDec(root->child);
            break;
        }
    }
}

void TranslateDec(Node *root) {
    //Dec -> VarDec | VarDec ASSIGNOP Exp;
    //printf("%s\n", __FUNCTION__);
    if(!root)
        return;
    switch(root->opt) {
        case 1: 
            TranslateVarDec(root->child);
            return;
        case 2: { 
            if(root->child->opt == 1) { // VarDec -> ID
                Operand *new_op = (Operand*)malloc(sizeof(Operand));
                new_op->kind = VAR;
                new_op->u.var_no = ++var_n;
                Symbol *sym = FindVarSymbolByName(sym_table, root->child->child->value, 100);
                sym->u.variable->op = new_op;

                InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
                InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
                new_code->kind = ASSIGN;
                new_code->u.assign.left = new_op;

                // create temp op
                Operand *temp = CreateTemp();
                TranslateExp(root->child->next->next, temp);
                new_code->u.assign.right = temp;
                    
                new_codes->code = new_code;
                new_codes->prev = NULL;
                new_codes->next = NULL;
                    
                AppendCodes(new_codes);
                return;
            }
            else { /* //TODO: 数组的定义/初始化; VarDec -> VarDec LB INT RB; 不考虑高维数组,VarDec2 -> ID
                Operand *temp_op1 = CreateTemp(); 
                Operand *temp_op2 = CreateTemp();
                CACalBase(root->child->next->next)
                CACalOffset(root->child->next->next->value, temp_op);

                Operand *new_op = (Operand*)malloc(sizeof(Operand));
                new_op->kind = VAR;
                new_op->u.var_no = ++var_n;

                InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
                InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
                new_code->kind = POINT_ASSIGN;
                new_code->u.assign.left = new_op;
                return;
                */
            }
            
        }
        default:
            break;
    }
}

void TranslateVarDec(Node *root) {
    //VarDec -> ID | VarDec LB INT RB; Var definition without assignment
    switch(root->opt) {
        case 2: {
            if(root->child->opt == 1) {
                root = root->child;
                unsigned size = GetSizeOfArray(root->child);
                Operand *new_op = (Operand*)malloc(sizeof(Operand));
                new_op->kind = VAR;
                new_op->u.var_no = ++var_n;
                Symbol *sym = FindVarSymbolByName(sym_table, root->child->value, 100);
                sym->u.variable->op = new_op;

                InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
                InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
                new_code->kind = DEC;
                new_code->u.dec.op = new_op;
                new_code->u.dec.size = size;
                    
                new_codes->code = new_code;
                new_codes->prev = NULL;
                new_codes->next = NULL;
                    
                AppendCodes(new_codes);
                return;
            }
            else {
                fprintf(stderr, ERROR);
                exit(-1);
            }
        }
        default:
            break;
    }
}

void TranslateStmtList(Node *root) {
    if(!root)
        return;
    //StmtList -> Stmt StmtList;
    TranslateStmt(root->child);
    TranslateStmtList(root->child->next);
}

void TranslateStmt(Node *root) {
    if(!root) {
        fprintf(stderr, "%s: statement is NULL!\n", __FUNCTION__);
        exit(-1);
    }
    switch(root->opt) {
        case 1: // Stmt -> Exp SEMI;
            TranslateExp(root->child, NULL);
            return;
        case 2: // Stmt -> CompSt;
            TranslateCompSt(root->child);
            return;
        case 3: { // Stmt -> RETURN Exp SEMI; 
            Operand *temp_op = CreateTemp();
            TranslateExp(root->child->next, temp_op);

            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));

            new_code->kind = RETURNC;
            new_code->u.sin.op = temp_op;

            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            AppendCodes(new_codes);
            return;
        }
        case 7: { // Stmt -> IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
            //new label1
            Operand *label1 = CreateLabel();

            //new label2
            Operand *label2 = CreateLabel();

            //顺序：Cond label1 Stmt label2
            TranslateCond(root->child->next->next, label1, label2);
            AppendLabel(label1);
            TranslateStmt(root->child->next->next->next->next);
            AppendLabel(label2);

            return;

        }
        case 8: { // Stmt -> IF LP Exp RP Stmt ELSE Stmt
            if(root->child->next->next->next->next->opt != 3) {
                //new label1
                Operand *label1 = CreateLabel();

                //new label2
                Operand *label2 = CreateLabel();

                //new label3
                Operand *label3 = CreateLabel();

                //顺序：Cond label1 Stmt1 GOTO_label3 label2 Stmt2 label3
                TranslateCond(root->child->next->next, label1, label2);
                AppendLabel(label1);
                TranslateStmt(root->child->next->next->next->next);

                
                //append GOTO label3
                InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
                InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
                new_code->kind = GOTO;
                new_code->u.sin.op = label3;
                new_codes->code = new_code;
                new_codes->next = new_codes->prev = NULL;
                AppendCodes(new_codes);

                AppendLabel(label2);
                TranslateStmt(root->child->next->next->next->next->next->next);

                AppendLabel(label3);

                return;
            }
            else {
                Operand *label1 = CreateLabel();

                //new label2
                Operand *label2 = CreateLabel();
                TranslateCond(root->child->next->next, label1, label2);
                AppendLabel(label1);
                TranslateStmt(root->child->next->next->next->next);
                AppendLabel(label2);
                TranslateStmt(root->child->next->next->next->next->next->next);
                return;
            }

        }
        case 9: { // Exp -> WHILE LP Exp RP Stmt
            //new label1
            Operand *label1 = CreateLabel();

            //new label2
            Operand *label2 = CreateLabel();

            //new label3
            Operand *label3 = CreateLabel();

            //顺序：label1 Cond label2 stmt GOTO_label1 label3
            AppendLabel(label1);
            TranslateCond(root->child->next->next, label2, label3);
            AppendLabel(label2);
            TranslateStmt(root->child->next->next->next->next);
            //GOTO_label1
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = GOTO;
            new_code->u.sin.op = label1;
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            AppendCodes(new_codes);

            AppendLabel(label3);
            break;
        }
        default:
            fprintf(stderr, "%s: Wrong opt %d of statement!\n", __FUNCTION__, root->opt);
            break;
    }
}


void TranslateExp(Node *root, Operand *place) {
    if(!root)
        return;
    switch(root->opt) {
        case 1: { //Exp->Exp ASSIGNOP Exp
            //printf("Exp opt %d\n", root->opt);
            if(root->child->opt == 16) { //Exp1->ID 
                Operand *temp_op = CreateTemp();
                TranslateExp(root->child->next->next, temp_op);
                Symbol *sym = FindVarSymbolByName(sym_table, root->child->child->value, 100);
                if(!sym->u.variable->op) {
                    sym->u.variable->op = CreateVar();
                } 
                InterCode *new_code1 = (InterCode*)malloc(sizeof(InterCode));
                InterCodes *new_codes1 = (InterCodes*)malloc(sizeof(InterCodes));
                new_code1->kind = ASSIGN;
                new_code1->u.assign.left = sym->u.variable->op;
                new_code1->u.assign.right = temp_op;
                new_codes1->code = new_code1;
                new_codes1->next = new_codes1->prev = NULL;
                AppendCodes(new_codes1);

                if(place == NULL)
                    return;
                InterCode *new_code2 = (InterCode*)malloc(sizeof(InterCode));
                InterCodes *new_codes2 = (InterCodes*)malloc(sizeof(InterCodes));
                new_code2->kind = ASSIGN;
                new_code2->u.assign.left = place;
                new_code2->u.assign.right = sym->u.variable->op;
                new_codes2->code = new_code2;
                new_codes2->next = new_codes2->prev = NULL;
                AppendCodes(new_codes2);
            }
            else if (root->child->opt == 15) {
                Operand *t0 = CreateTemp();
                TranslateExp(root->child->next->next, t0);
                root = root->child;
                if(root->child->opt == 16) {
                    Operand *t1 = CreateTemp();
                    TranslateExp(root->child->next->next, t1);
                    Symbol *sym = FindVarSymbolByName(sym_table, root->child->child->value, 100);
                    if(!sym || sym->u.variable->type->kind !=ARRAY) {
                        fprintf(stderr, "%s: We have a wrong array!\n", __FUNCTION__);
                        exit(-1);
                    }
                    int elem_size = CalculateSize(sym->u.variable->type->u.array.elem);
                    Operand *size = CreateConst(elem_size);
                    Operand *t2 = CreateTemp();
                    Operand *t3 = CreateTemp();
                    Operand *t4 = CreateTemp();
                    CAAddrCodes(t2, sym->u.variable->op);
                    CAMulCodes(t3, t1, size);
                    CAAddCodes(t4, t2, t3);
                    CAPointAssignCodes(t4, t0);
                }
                else {
                    fprintf(stderr, ERROR);
                    exit(-1);
                }
            }
            return;
        }
        case 3: { //Exp->Exp PLUS Exp
            //printf("Exp opt %d\n", root->opt);
            Operand *temp_op1 = CreateTemp(), *temp_op2 = CreateTemp();
            TranslateExp(root->child, temp_op1);
            TranslateExp(root->child->next->next, temp_op2);


            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = ADD;
            new_code->u.binop.res = place;
            new_code->u.binop.op1 = temp_op1;
            new_code->u.binop.op2 = temp_op2;
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;

            AppendCodes(new_codes);
            return;
        }
        case 4: { //Exp->Exp MINUS Exp
            //printf("Exp opt %d\n", root->opt);
            Operand *temp_op1 = CreateTemp(), *temp_op2 = CreateTemp();
            TranslateExp(root->child, temp_op1);
            TranslateExp(root->child->next->next, temp_op2);


            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = SUB;
            new_code->u.binop.res = place;
            new_code->u.binop.op1 = temp_op1;
            new_code->u.binop.op2 = temp_op2;
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            
            AppendCodes(new_codes);
            return;
        }
        case 5: { //Exp->MINUS Exp
            //printf("Exp opt %d\n", root->opt);
            Operand *temp_op = CreateTemp();
            TranslateExp(root->child->next, temp_op);
            
            Operand *zero = CreateConst(0);
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = SUB;
            new_code->u.binop.res = place;
            new_code->u.binop.op1 = zero;
            new_code->u.binop.op2 = temp_op;
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;

            AppendCodes(new_codes);
            return;
        }
        case 6: { //Exp->Exp STAR Exp
            //printf("Exp opt %d\n", root->opt);
            Operand *temp_op1 = CreateTemp(), *temp_op2 = CreateTemp();
            TranslateExp(root->child, temp_op1);
            TranslateExp(root->child->next->next, temp_op2);


            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = MUL;
            new_code->u.binop.res = place;
            new_code->u.binop.op1 = temp_op1;
            new_code->u.binop.op2 = temp_op2;
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            
            AppendCodes(new_codes);
            return;
        }
        case 7: { //Exp->Exp DIV Exp
            //printf("Exp opt %d\n", root->opt);
            Operand *temp_op1 = CreateTemp(), *temp_op2 = CreateTemp();
            TranslateExp(root->child, temp_op1);
            TranslateExp(root->child->next->next, temp_op2);


            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = DIVC;
            new_code->u.binop.res = place;
            new_code->u.binop.op1 = temp_op1;
            new_code->u.binop.op2 = temp_op2;
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            
            AppendCodes(new_codes);
            return;
        }
        case 2: //Exp->Exp RELOP Exp
        case 8: //Exp->Exp AND Exp
        case 9: //Exp->Exp OR Exp
        case 10: {   //Exp->NOT Exp
            //printf("Exp opt %d\n", root->opt);
            Operand *label1 = CreateLabel();
            Operand *label2 = CreateLabel();
        
            CAConstCodes(place, 0);
            TranslateCond(root, label1, label2);
            AppendLabel(label1);
            CAConstCodes(place, 1);
            AppendLabel(label2);
            return;
        }
        case 11:   //Exp->LP Exp RP 
            //printf("Exp opt %d\n", root->opt);
            TranslateExp(root->child->next, place);
            return;
        case 12:    { //Exp->ID LP RP
            //printf("Exp opt %d\n", root->opt);
            Symbol *sym = FindFunSymbolByName(sym_table, root->child->value);
            Operand *op;
            if(!sym->u.function->op) {
                op = CreateFun(root->child->value);
                sym->u.function->op = op;
            }
            else
                op = sym->u.function->op;
            if(strcmp(sym->name, "read") == 0) {
                InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
                InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
                new_code->kind = READ;
                new_code->u.sin.op = place;
                new_codes->code = new_code;
                new_codes->next = new_codes->prev = NULL;
                AppendCodes(new_codes);
                return;
            }
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = CALL;
            new_code->u.sin.op = op;
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            AppendCodes(new_codes);
            return;
        }
        case 13: { //Exp->ID LP Args RP
            //printf("Exp opt %d\n", root->opt);
            int i;
            Symbol *sym = FindFunSymbolByName(sym_table, root->child->value);
            Operand *op;
            if(!sym->u.function->op) {
                op = CreateFun(root->child->value);
                sym->u.function->op = op;
            }
            else
                op = sym->u.function->op;
            Operand* arg_list[sym->u.function->argc];
            TranslateArgs(root->child->next->next, arg_list, 0);
            if(strcmp(root->child->value, "write") == 0) {
                InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
                InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
                new_code->kind = WRITE;
                new_code->u.sin.op = arg_list[0];

                new_codes->code = new_code;
                new_codes->next = new_codes->prev = NULL;
                AppendCodes(new_codes);

                if(place)
                    CAConstCodes(place, 0);
                return;
            }
            i = sym->u.function->argc;
            while(i > 0 ) {
                CAArgCodes(arg_list[--i]);
            }
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = CALL;
            new_code->u.sinop.res = place;
            new_code->u.sinop.op1 = op;
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            AppendCodes(new_codes);
            return;

        }
        case 14: { //Exp->Exp DOT ID; 本实验不考虑结构体
            //printf("Exp opt %d\n", root->opt);
            fprintf(stderr, ERROR);
            exit(-1);
        }
        case 15:  { //Exp->Exp LB Exp RB
            //printf("Exp opt %d\n", root->opt);
            if(root->child->opt == 16) {
                Operand *t1 = CreateTemp();
                TranslateExp(root->child->next->next, t1);
                Symbol *sym = FindVarSymbolByName(sym_table, root->child->child->value, 100);
                if(!sym || sym->u.variable->type->kind !=ARRAY) {
                    fprintf(stderr, "%s: We have a wrong array!\n", __FUNCTION__);
                    exit(-1);
                }
                int elem_size = CalculateSize(sym->u.variable->type->u.array.elem);
                Operand *size = CreateConst(elem_size);
                Operand *t2 = CreateTemp();
                Operand *t3 = CreateTemp();
                Operand *t4 = CreateTemp();
                CAAddrCodes(t2, sym->u.variable->op);
                CAMulCodes(t3, t1, size);
                CAAddCodes(t4, t2, t3);
                if(place == NULL)
                    return;
                CAPointCodes(place, t4);
                return;
                
            }
            else {
                fprintf(stderr, ERROR);
                exit(-1);
            }
            return;
        }
        case 16: { //Exp->ID
            //printf("Exp opt %d\n", root->opt);
            if(!place)
                return;
            Symbol *sym  = FindVarSymbolByName(sym_table, root->child->value, 100);
            Operand *new_op;
            if(sym->u.variable->op) 
                new_op = sym->u.variable->op;
            else {
                new_op = CreateVar();
                sym->u.variable->op = new_op;
            }
    
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = ASSIGN;
            new_code->u.assign.left = place;
            new_code->u.assign.right = new_op;

            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            AppendCodes(new_codes);
            return;
        }
        case 17:  { //Exp->INT
            //printf("Exp opt %d\n", root->opt);
            if(place == NULL) {
                return;
            }
            Operand *new_op = (Operand*)malloc(sizeof(Operand));
            new_op->kind = CONSTANT;
            new_op->u.value = atoi(root->child->value); //TOTEST
            //printf("INT: %d\n", new_op->u.value);

            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = ASSIGN;
            new_code->u.assign.left = place;
            new_code->u.assign.right = new_op;

            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            AppendCodes(new_codes);
            return;
        }
        case 18:  //Exp->FLOAT; 本实验不考虑浮点数常量
            fprintf(stderr, "Cannot translate: in %s, We hava a float constant!\n", __FUNCTION__);
            exit(-1);
        default:
            break;
    }
}

void TranslateArgs(Node *root, Operand **arg_list, int index) {
    // Args -> Exp | Exp COMMA Args
    if(!root)
        return;
    switch(root->opt) {
        case 1: { // Args -> Exp COMMA Args
            Operand *temp_op = CreateTemp();
            TranslateExp(root->child, temp_op);
            arg_list[index++] = temp_op;
            TranslateArgs(root->child->next->next, arg_list, index);
            return;
        }
        case 2: { // Args -> Exp 
            if(root->child->opt != 16) {
                Operand *temp_op = CreateTemp();
                TranslateExp(root->child, temp_op);
                //printf("TEST: %d\n", root->child->opt);
                arg_list[index++] = temp_op;
                return;
            }
            else {
                Symbol *sym  = FindVarSymbolByName(sym_table, root->child->child->value, 100);
                Operand *new_op;
                if(sym->u.variable->op) 
                    new_op = sym->u.variable->op;
                else {
                    new_op = CreateVar();
                    sym->u.variable->op = new_op;
                }
                //printf("TEST: %d\n", root->child->opt);
                arg_list[index++] = new_op;
                return;
            }
        }
    }
}

void TranslateCond(Node *root, Operand *label1, Operand *label2) {
    switch(root->opt) {
        case 2: { //Exp->Exp RELOP Exp
            Operand *temp_op1 = CreateTemp(), *temp_op2 = CreateTemp();
            TranslateExp(root->child, temp_op1);
            TranslateExp(root->child->next->next, temp_op2);
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = COND;
            new_code->u.cond.op1 = temp_op1;
            new_code->u.cond.op2 = temp_op2;
            new_code->u.cond.target = label1;
            strcpy(new_code->u.cond.relop, root->child->next->value);
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            AppendCodes(new_codes);
            CAGotoCodes(label2);
            return;
        }
        case 8: { //Exp->Exp AND Exp
            Operand *label3 = CreateLabel();
            TranslateCond(root->child, label3, label2);
            AppendLabel(label3);
            TranslateCond(root->child->next->next, label1, label2);
        }
        case 9: { //Exp->Exp OR Exp
            Operand *label3 = CreateLabel();
            TranslateCond(root->child, label1, label3);
            AppendLabel(label3);
            TranslateCond(root->child->next->next, label1, label2);
            return;
        }
        case 10: {   //Exp->NOT Exp
            TranslateCond(root->child->next, label2, label1);
            return;
        }
        default: {
            Operand *op1 = CreateTemp();
            TranslateExp(root, op1);
            Operand *op2 = CreateConst(0);
            InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
            InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
            new_code->kind = COND;
            new_code->u.cond.op1 = op1;
            new_code->u.cond.op2 = op2;
            new_code->u.cond.target = label1;
            strcpy(new_code->u.cond.relop, "!=");
            new_codes->code = new_code;
            new_codes->next = new_codes->prev = NULL;
            AppendCodes(new_codes);
            CAGotoCodes(label2);
            return;
        }
    }
}

unsigned GetSizeOfArray(Node *root) { // calculate size of array with Symbolm root = ID
    Symbol *sym = FindVarSymbolByName(sym_table, root->value, 100);
    if( sym->kind != VARIABLE ) {
        fprintf(stderr, "%s: This is not an array!\n", __FUNCTION__);
        exit(-1);
    }
    return CalculateSize(sym->u.variable->type);
}

unsigned CalculateSize(Type *type) { // calculate size of array with Type
    switch(type->kind) {
        case BASIC_INT:
            return 4;
        case BASIC_FLOAT:
            return 8;
        case ARRAY: 
            return type->u.array.size * CalculateSize(type->u.array.elem);
        default:
            fprintf(stderr, "%s: We have a structure!\n", __FUNCTION__);
            exit(-1);
    }
}

void AppendCodes(InterCodes *codes) {
    if(head == NULL) {
        fprintf(stderr, "%s: Append to NULL!\n", __FUNCTION__);
        exit(-1);
    }
    InterCodes *cur = head;
    while(cur->next)
        cur = cur->next;
    cur->next = codes;
    codes->prev = cur;
    return;
}

InterCodes* InitCodes() {
    InterCodes *head = (InterCodes*)malloc(sizeof(InterCodes));
    InterCode *code = (InterCode*)malloc(sizeof(InterCode));
    // indicate head of Codes List
    code->kind = HEAD;
    code->u.sin.op = NULL;

    head->code = code;
    head->next = NULL;
    head->prev = NULL;
    
    return head;
}

void AppendLabel(Operand *label) {
    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));

    new_code->kind = LABELC;
    new_code->u.sin.op = label;

    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
}

Operand* CreateLabel() {
    Operand *label = (Operand*)malloc(sizeof(Operand));
    label->kind = LABEL;
    label->u.label_no = ++label_n;

    return label;
}

Operand* CreateTemp() {
    Operand *temp = (Operand*)malloc(sizeof(Operand));
    temp->kind = TEMP;
    temp->u.temp_no = ++temp_n;
    
    return temp;
}

void CAConstCodes(Operand *left, int n) {
    Operand *new_op = CreateConst(n);

    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));

    new_code->kind = ASSIGN;
    new_code->u.assign.left = left;
    new_code->u.assign.right = new_op;

    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
}

Operand *CreateConst(int n) {
    Operand *op = (Operand*)malloc(sizeof(Operand));
    op->kind = CONSTANT;
    op->u.value = n;

    return op;
}

void CAArgCodes(Operand *arg) {
    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
    new_code->kind = ARG;
    new_code->u.sin.op = arg;
    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
    return;
}

void CAGotoCodes(Operand *label) {
    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
    new_code->kind = GOTO;
    new_code->u.sin.op = label; 
    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
}


void PrintCodes(InterCodes *head) {
    if(!head)
        return;
    InterCodes *cur = head->next;
    while(cur) {
        switch(cur->code->kind) {
            case ASSIGN:
                PrintOp(cur->code->u.assign.left);
                printf(" := ");
                PrintOp(cur->code->u.assign.right);
                printf("\n");
                break;
            case ADD:
                PrintOp(cur->code->u.binop.res);
                printf(" := ");
                PrintOp(cur->code->u.binop.op1);
                printf(" + ");
                PrintOp(cur->code->u.binop.op2);
                printf("\n");
                break;
            case SUB:
                PrintOp(cur->code->u.binop.res);
                printf(" := ");
                PrintOp(cur->code->u.binop.op1);
                printf(" - ");
                PrintOp(cur->code->u.binop.op2);
                printf("\n");
                break;
            case MUL:
                PrintOp(cur->code->u.binop.res);
                printf(" := ");
                PrintOp(cur->code->u.binop.op1);
                printf(" * ");
                PrintOp(cur->code->u.binop.op2);
                printf("\n");
                break;
            case DIVC:
                PrintOp(cur->code->u.binop.res);
                printf(" := ");
                PrintOp(cur->code->u.binop.op1);
                printf(" / ");
                PrintOp(cur->code->u.binop.op2);
                printf("\n");
                break;
            case ADDR:
                PrintOp(cur->code->u.sinop.res);
                printf(" := &");
                PrintOp(cur->code->u.sinop.op1);
                printf("\n");
                break;
            case POINT:
                PrintOp(cur->code->u.sinop.res);
                printf(" := *");
                PrintOp(cur->code->u.sinop.op1);
                printf("\n");
                break;
            case POINT_ASSIGN:
                printf("*");
                PrintOp(cur->code->u.sinop.res);
                printf(" := ");
                PrintOp(cur->code->u.sinop.op1);
                printf("\n");
                break;
            case FUNC:
                printf("FUNCTION ");
                PrintOp(cur->code->u.sin.op);
                printf(" :\n");
                break;
            case LABELC:
                printf("LABEL ");
                PrintOp(cur->code->u.sin.op);
                printf(" :\n");
                break;
            case PARAMC:
                printf("PARAM ");
                PrintOp(cur->code->u.sin.op);
                printf("\n");
                break;
            case DEC:
                printf("DEC ");
                PrintOp(cur->code->u.dec.op);
                printf(" %d\n", cur->code->u.dec.size);
                break;
            case RETURNC:
                printf("RETURN ");
                PrintOp(cur->code->u.sin.op);
                printf("\n");
                break;
            case GOTO:
                printf("GOTO ");
                PrintOp(cur->code->u.sin.op);
                printf("\n");
                break;
            case COND:
                printf("IF ");
                PrintOp(cur->code->u.cond.op1);
                printf(" %s ", cur->code->u.cond.relop);
                PrintOp(cur->code->u.cond.op2);
                printf(" GOTO ");
                PrintOp(cur->code->u.cond.target);
                printf("\n");
                break;
            case CALL:
                PrintOp(cur->code->u.sinop.res);
                printf(" := CALL ");
                PrintOp(cur->code->u.sinop.op1);
                printf("\n");
                break;
            case ARG:
                printf("ARG ");
                PrintOp(cur->code->u.sin.op);
                printf("\n");
                break;
            case READ:
                printf("READ ");
                PrintOp(cur->code->u.sin.op);
                printf("\n");
                break;
            case WRITE:
                printf("WRITE ");
                PrintOp(cur->code->u.sin.op);
                printf("\n");
                break;
            default:
                fprintf(stderr, "%s: We found an intercodes of non-existing kind!\n", __FUNCTION__);
                exit(-1);
        }
        cur = cur->next;
    }
}

void PrintOp(Operand *op) {
    if(!op)
        return;
    switch(op->kind) {
        case VAR:
            printf("v%d", op->u.var_no);
            break;
        case CONSTANT:
            printf("#%d", op->u.value);
            break;
        case LABEL:
            printf("label%d", op->u.label_no);
            break;
        case FUN:
            printf("%s", op->u.name);
            break;
        case HEAD:
            break;
        case TEMP:
            printf("t%d", op->u.temp_no);
            break;
    }
}

void SetEnv(HashTable *table) {
    //添加read
    Type *type = (Type*)malloc(sizeof(Type));
    type->kind = BASIC_INT;
    type->u.int_value = 0;
    Function* fun1 = (Function*)malloc(sizeof(Function)); 
    fun1->ret_type = type; 
    fun1->op = NULL;
    fun1->argc = 0;
    fun1->args = NULL;
    Symbol *newSym1 = CreateFunSymbol("read", fun1);
    
    //添加write函数
    Function* fun2 = (Function*)malloc(sizeof(Function)); 
    fun2->ret_type = type; 
    fun2->op = NULL;
    fun2->argc = 1;
    FieldList *arg = (FieldList*)malloc(sizeof(FieldList));
    arg->name = (char*)malloc(sizeof(char) * MAX_LENGTH);
    strcpy(arg->name, " "); //write形参占位符
    arg->type = type;
    arg->tail = NULL;
    fun2->args = arg;
    Symbol *newSym2 = CreateFunSymbol("write", fun2);
    InsertHash(table, newSym1);
    InsertHash(table, newSym2);
}

Operand* CreateVar() {
    Operand *op = (Operand*)malloc(sizeof(Operand));
    op->kind = VAR;
    op->u.var_no = ++var_n;

    return op;
}

Operand* CreateFun(char *name) {
    Operand *op = (Operand*)malloc(sizeof(Operand));
    op->kind = FUN;
    op->u.name = (char*)malloc(sizeof(char) * MAX_LENGTH);
    strcpy(op->u.name, name);

    return op;
}

void CAMulCodes(Operand *res, Operand *op1, Operand *op2) {
    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
    
    new_code->kind = MUL;
    new_code->u.binop.res = res;
    new_code->u.binop.op1 = op1;
    new_code->u.binop.op2 = op2;

    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
}

void CAAddrCodes(Operand *left, Operand *right) {
    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
    
    new_code->kind = ADDR;
    new_code->u.sinop.res = left;
    new_code->u.binop.op1 = right;

    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
}

void CAAddCodes(Operand *res, Operand *op1, Operand *op2) {
    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
    
    new_code->kind = ADD;
    new_code->u.binop.res = res;
    new_code->u.binop.op1 = op1;
    new_code->u.binop.op2 = op2;

    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
}

void CAPointCodes(Operand *left, Operand *right) {
    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
    
    new_code->kind = POINT;
    new_code->u.sinop.res = left;
    new_code->u.binop.op1 = right;

    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
}

void CAPointAssignCodes(Operand *left, Operand *right) {
    InterCode *new_code = (InterCode*)malloc(sizeof(InterCode));
    InterCodes *new_codes = (InterCodes*)malloc(sizeof(InterCodes));
    
    new_code->kind = POINT_ASSIGN;
    new_code->u.sinop.res = left;
    new_code->u.binop.op1 = right;

    new_codes->code = new_code;
    new_codes->next = new_codes->prev = NULL;
    AppendCodes(new_codes);
}

void GenIR(char *filename, InterCodes *head) {
    freopen(filename, "w", stdout);
    PrintCodes(head);
    fclose(stdout);
    return;
}