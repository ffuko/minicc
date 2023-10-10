#ifndef _TREE_H
#define _TREE_H


#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define DEFAULT 40

typedef struct node{
    int line;
    char name[DEFAULT];
    char value[DEFAULT];
    struct node* child;
    struct node* next;
    int opt; //case of production for each non-terminal
}Node;


Node* createNode(char *name, char *value, int line, int opt);
void addChild(Node *parent, Node *child);
void printTree(Node *root, int times);
int checkType(char *name);

#endif
