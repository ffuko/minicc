#include "tree.h"
#include <stdio.h>
#include "syntax.tab.h"
#include "symbol-table.h"
#include "semantic.h"
#include "intermediate.h"

//#define YYDEBUG 1

extern Node* root;
extern void yyrestart();

int errorOccur = 0;
int var_n = 0;
int temp_n = 0;
int label_n = 0;

HashTable *sym_table;
InterCodes *head; // head of interCodes

int main(int argc, char **argv) {
  if(argc <=1 ) {
    printf("No file to parse!\n");
    return 1;
  }
  else if(argc <= 2) {
    printf("No IR file specified!\n");
    return 1;
  }
  FILE* f = fopen(argv[1], "r");
  if (!f) {
    perror(argv[1]);
    return 1;
  }

  yyrestart(f);
  yyparse();
  if(!errorOccur){
    sym_table = InitHashTable();
    //printTree(root, 0);
    SetEnv(sym_table);
    Program(root);
    head = InitCodes();
    TranslateProgram(root);
    //PrintCodes(head);
    GenIR(argv[2], head);
  }
  else 
    exit(-1);
  return 0;
}