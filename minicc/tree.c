#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PREFIX 100
#define equal(a, b, c) strcmp(a, b) == 0 \
                    || strcmp(a, c) == 0 \

Node* createNode(char *name, char *value, int line, int opt) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->line = line;
    newNode->child = newNode->next = NULL;
    newNode->opt = opt;
    strcpy(newNode->name, name);
    strcpy(newNode->value, value); 
    return newNode;
}


void addChild(Node *parent, Node *child) {
    if(parent == NULL || child == NULL)
        return;
    else if(parent->child == NULL)
        parent->child = child;
    else {
        Node *firstChild = parent->child;
        while(firstChild->next)
            firstChild = firstChild->next;
        firstChild->next = child;
    }   
    //printf("child: %s\n", child->value);
    return;    
}


void printTree(Node *root, int times) {
    Node* cur = root;
    if(cur) {
        char prefix[MAX_PREFIX];
        memset(prefix, ' ', times);
        prefix[times] = '\0';
        if(cur->value[0] == '\0') 
            printf("%s%s (%d)\n", prefix, cur->name, cur->line);
        else if (checkType(cur->name)) //if cur->name == [INT/FLOAT/ID/TYPE] ,print its value
            printf("%s%s: %s\n", prefix, cur->name, cur->value);
        else if (strcmp(cur->name, "INT") == 0)
            printf("%s%s: %d\n", prefix, "INT", atoi(cur->value));
        else if (strcmp(cur->name, "OCT") == 0) // print oct number
            printf("%s%s: %d\n", prefix, "INT", strtol(cur->value, NULL, 8));
        else if (strcmp(cur->name, "HEX") == 0) // print hex number
            printf("%s%s: %d\n", prefix, "INT", strtol(cur->value, NULL, 16));
        else if (strcmp(cur->name, "INDEX") == 0) // print index form of float
            printf("%s%s: %f\n", prefix, "FLOAT", strtod(cur->value, NULL));
        else if(strcmp(cur->name, "FLOAT") == 0)
            printf("%s%s: %f\n", prefix, "FLOAT", atof(cur->value));
        else 
            printf("%s%s\n", prefix, cur->name);

        if(cur->child)
            printTree(cur->child, times+2);
        
        if(cur->next) {
            printTree(cur->next, times);
        }
    }   
    return;
}


inline int checkType(char *name) {
    if(equal(name, "TYPE", "ID"))   
        return 1;
    return 0;
}