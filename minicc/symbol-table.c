#include "symbol-table.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

extern HashTable *sym_table;

HashTable* InitHashTable() {
    //printf("%s\n", __FUNCTION__);
    HashTable *table = (HashTable*)malloc(sizeof(HashTable));
    if(!table)
        return NULL;
    table->count = 0;
    table->symbols = (ListNode*)malloc(HASH_SIZE * (sizeof(ListNode)));

    for(int i=0; i< HASH_SIZE; i++)
        table->symbols[i] = NULL;
    
    return table;
}

unsigned int Hash(char *name) { //pjw hash funcion
    //printf("%s\n", __FUNCTION__);
    unsigned int val = 0, i;
    for(; *name; name++) {
        val = (val << 2) + *name;
        if(i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
    } 
    return val;
}

Status SearchHash(HashTable *table, ListNode node) {
    //printf("%s\n", __FUNCTION__);
    // p: index of searched elem; c:  number of conflict
    unsigned int p = Hash(node->name);
    if(table->symbols[p] != NULL &&  // searched record exists
        SearchNode(table->symbols[p], node))//and keys equal
        return STATE_SUCCESS;  
    else
        return STATE_FAILURE;
}

Status InsertHash(HashTable *table, ListNode node) {
    //printf("%s\n", __FUNCTION__);
    int c = 0;
    int p = Hash(node->name);
    if(SearchHash(table, node))
        return STATE_FAILURE; // element with same key exisits
    else {
        AppendNode(table, p, node);
        table->count++;
        return STATE_SUCCESS;
    }
}

Status SearchNode(ListNode head, ListNode node) {
    //printf("%s\n", __FUNCTION__);
    if(!head)
        return STATE_FAILURE;
    ListNode cur = head;
    while(cur != NULL) {
        if(CheckEqualListNode(cur, node))
            return STATE_SUCCESS;
        cur = cur->next;
    }
    return STATE_FAILURE;
}

Status AppendNode(HashTable *table, int p, ListNode node) {
    //printf("%s\n", __FUNCTION__);
    if(!table->symbols[p]) {
        table->symbols[p] = node;
        node->next = NULL;
        return STATE_SUCCESS;
    }

    //if to-be-appended node/name exists, return failure 
    //if(SearchNode(head, node))
        //return STATE_FAILURE;
    
    ListNode cur = table->symbols[p];
    while(cur->next != NULL)
        cur = cur->next;
    cur->next = node;
    node->next = NULL;

    return STATE_SUCCESS;
}

Symbol* CreateFunSymbol(char *name, Function *function) {
    //printf("%s\n", __FUNCTION__);
    Symbol* newSym = (Symbol*)malloc(sizeof(Symbol));
    
    newSym->name = (char*)malloc(NAME_SIZE);
    strcpy(newSym->name, name);
    newSym->kind = FUNCTION;
    newSym->u.function = function;
    newSym->next = NULL;
    //printf("New Symbol: %s, Type: %d\n", newSym->name, newSym->kind);
    return newSym;
}

Symbol* CreateVarSymbol(char *name, Variable *variable){
    //printf("%s\n", __FUNCTION__);
    Symbol* newSym = (Symbol*)malloc(sizeof(Symbol));
    newSym->name = (char*)malloc(NAME_SIZE);
    strcpy(newSym->name, name);
    newSym->kind = VARIABLE;
    newSym->u.variable = variable;
    newSym->next = NULL;
    //printf("New Symbol: %s, Type: %d\n", newSym->name, newSym->kind);
    return newSym;
}

Symbol* FindVarSymbolByName(HashTable *table, char *name, int scope) {
    int p = Hash(name);
    ListNode head = table->symbols[p]; 
    if(!head)
        return NULL;
    while(head) {
        if(head->kind == VARIABLE && scope >= head->u.variable->scope)
            return head;
        head = head->next;
    }
    return NULL;
}

Symbol* FindFunSymbolByName(HashTable *table, char *name) {
    int p = Hash(name);
    ListNode head = table->symbols[p]; 
    if(!head)
        return NULL;
    while(head) {
        if(head->kind == FUNCTION)
            return head;
        head = head->next;
    }
    return NULL;
}


Status CheckEqualListNode(ListNode cur, ListNode node) {
    //printf("%s\n", __FUNCTION__);
    if(strcmp(cur->name,node->name) == 0) {
        switch(cur->kind) {
            case VARIABLE:
                if(node->kind == VARIABLE && cur->u.variable->scope == node->u.variable->scope)
                    return STATE_SUCCESS;
                else if(cur->u.variable->type->kind == STRUCTURE || node->u.variable->type->kind == STRUCTURE)
                    return STATE_SUCCESS;
                break;
            case FUNCTION:
                return STATE_SUCCESS;

        }
    } 
    return STATE_FAILURE;
}

Status PrintTable(HashTable *table) {
    //printf("%s\n", __FUNCTION__);
    for(int i=0; i<HASH_SIZE; i++) {
        Symbol *cur = table->symbols[i];
        while(cur != NULL) {
            if(cur->kind == VARIABLE)
                printf("symbol %d: %s, scope: %d\n", i, table->symbols[i]->name, table->symbols[i]->u.variable->scope);
            else 
                printf("symbol %d: %s\n", i, table->symbols[i]->name);
            cur = cur->next; 
        }
    }
    return STATE_SUCCESS;
}

Status check_type(Type *type1, Type *type2) {
    //if(type1 == NULL || type2 == NULL)
      //  return STATE_FAILURE;
    if(type1->kind != STRUCTURE  || type2->kind !=STRUCTURE) {
        if( type1->kind == type2->kind)
            return STATE_SUCCESS;
        else 
            return STATE_FAILURE;
    }
    

    FieldList *str1 = type1->u.structure->tail; //跳过标志头
    FieldList *str2 = type2->u.structure->tail;
    while(str1 && str2) {
        if(!check_type(str1->type, str2->type))
            return STATE_FAILURE;
        str1 = str1->tail;
        str2 = str2->tail; 
    }
    if(str1 == NULL && str2 == NULL)
        return STATE_SUCCESS;
    else 
        return STATE_FAILURE;
}