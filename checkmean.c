#include <string.h>
#include "common.h"

//third level
int dfsExtDec(struct treeNode* node,char* type){
    //array

    //normal

    //struct
    return 0;
}

int dfsFunDec(struct treeNode* node){

    return 0;
}
int dfsStructDec(struct treeNode* node){

    return 0;
}


//second level
int dfsExtDef(struct treeNode* node){
    if (node == NULL) return 0;
    struct treeNode* son = node->sonlist;
    //Specifier
    struct treeNode* sonson = son->sonlist;
    char *type = NULL; 
    if (strcmp(sonson->name,"TYPE") == 0){
        type = sonson->value;
    }else{
        //区分struct的定义 | struct定义变量
        type = type = sonson->sonlist->next->sonlist->name;
        if (strcmp(sonson->sonlist->next->name,"Tag") == 0){
            //struct 定义变量
        }else{
            dfsStructDec(sonson);
        }
    }

    //ExtDecList | FunDec 
    son = son->next;
    if (strcmp(son->name,"ExtDecList") == 0){
        dfsExtDec(son,type); 
    }

    if (strcmp(son->name,"FunDec") == 0){
        dfsFunDec(son); //{ insert in there to make xingcan after {
        son = son->next;
        if (strcmp(son->name,"SEMI") == 0){

        }else if (strcmp(son->name,"Compt") == 0){

        }
    }


    return 0;
}

//first level 
int dfsbuildtable(struct treeNode* node){
    if (node == NULL) return 0;
    if (strcmp(node->name,"ExtDef") == 0){
        dfsExtDef(node);
    }else{
        struct treeNode* son = node->sonlist;
        while(son != NULL){
            dfsbuildtable(son);
            son = son->next;
        }    
    }
    return 0;
}
int checkmean(struct treeNode* root){
    dfsbuildtable(root);
    
    return 0;
}
