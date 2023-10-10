#ifndef SYMBOL_TABLE_H__
#define SYMBOL_TABLE_H__


#define HASH_SIZE 0x3fff //size of hash table
#define MAX_LENGTH 32
#define NAME_SIZE (sizeof(char)*MAX_LENGTH)


typedef enum {
    STATE_FAILURE = 0,
    STATE_SUCCESS,
    STATE_DUPLICATE
}Status; // error code

enum {
    GLOBAL=1, PARAM, LOCAL
}; //value of scope, LOCAL+k if nested local scope; PARAM 结构体内部



typedef struct fieldList_ {
    char *name;
    struct type_ *type;
    struct fieldList_* tail; // next field
}FieldList;


typedef struct type_{
    enum { BASIC_INT = 0, BASIC_FLOAT, ARRAY, STRUCTURE } kind; 
    union {
        int int_value;
        float float_value;
        struct { struct type_* elem; int size; } array;
        FieldList* structure;//structure is a linked list
    }u; // info about type
}Type; //type of variable



typedef struct variable_ {
    Type* type;
    int scope;
    void *op;
    //unsigned int space; //内存空间大小
}Variable;

typedef struct function_ {
    Type* ret_type;
    int argc;
    FieldList* args;
    void *op;
}Function;

typedef struct symbol_{
    char *name;
    enum { VARIABLE = 1, FUNCTION } kind;
    union {
        Variable *variable;
        Function *function;
    }u;
    struct symbol_* next;
}Symbol;

typedef Symbol* ListNode;

typedef struct HashTable{
    ListNode* symbols; // symbol list
    unsigned int count; //number of valid symbols
}HashTable; //used as symbol table

typedef HashTable SymbolTable;

HashTable* InitHashTable();
unsigned int Hash(char *name);
Status SearchHash(HashTable *table, ListNode node);
Status InsertHash(HashTable *table, ListNode node);

Status SearchNode(ListNode head, ListNode node);
Status AppendNode(HashTable *table, int p, ListNode node);

Symbol* CreateFunSymbol(char *name, Function *function); //initialize new symbol 
Symbol* CreateVarSymbol(char *name, Variable *variable);

Symbol* FindVarSymbolByName(HashTable *table, char *name, int scope);
Symbol* FindFunSymbolByName(HashTable *table, char *name);

Status CheckEqualListNode(ListNode cur, ListNode node);

Status PrintTable(HashTable *table);
Status check_type(Type *type1, Type *type2);

#endif
