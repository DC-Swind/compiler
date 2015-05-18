#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

struct Instr *instrlist;
struct Varm* varmlist;
struct Structm *structlist;
int vnum = 0;
int tnum = 0;
int structN = 0;
int tmpN = 0;
int varN = 0;
int labelN = 0;

struct Instr* translate_DefList(struct Varm* list ,struct treeNode* node);
char* newVname();
struct Instr* translate_CompSt(struct treeNode* node);
//-----------------------------------------------------------------------------
int printInstrList(){
    FILE* file = fopen("output.rs","w+");
    if (file == NULL){
        printf("Can not open file.\n");
        return 0;
    }
    while(instrlist != NULL){
        switch(instrlist->type){
            case _LABEL:
                fprintf(file,"LABEL %s :\n",instrlist->arg1);
                break;
            case _FUNCTION:
                fprintf(file,"FUNCTION %s :\n",instrlist->arg1);
                break;
            case _ASSIGNOP:
                fprintf(file,"%s := %s\n",instrlist->target,instrlist->arg1);
                break;
            case _ADD:
                fprintf(file,"%s := %s + %s :\n",instrlist->target,instrlist->arg1,instrlist->arg2);
                break;
                
            case _MINUS:
                fprintf(file,"%s := %s - %s :\n",instrlist->target,instrlist->arg1,instrlist->arg2);
                break;
            case _STAR:
                fprintf(file,"%s := %s * %s :\n",instrlist->target,instrlist->arg1,instrlist->arg2);
                break;
                
            case _DIV:
                fprintf(file,"%s := %s / %s :\n",instrlist->target,instrlist->arg1,instrlist->arg2);
                break;
            case _GETADDR:
                fprintf(file,"%s := &%s\n",instrlist->target,instrlist->arg1);
                break;
            case _GETVALUE:
                fprintf(file,"%s := *%s\n",instrlist->target,instrlist->arg1);
                break;
            case _ASSIGNOP_GETVALUE:
                fprintf(file,"*%s := %s\n",instrlist->target,instrlist->arg1);
                break;
            case _GOTO:
                fprintf(file,"GOTO %s\n",instrlist->arg1);
                break;
            case _IF:
                fprintf(file,"IF %s %s %s GOTO %s\n",instrlist->arg1,instrlist->op,instrlist->arg2,instrlist->target);
                break; 
            case _RETURN:
                fprintf(file,"RETURN %s\n",instrlist->arg1);
                break;
            case _DEC:
                fprintf(file,"DEC %s %s\n",instrlist->arg1,instrlist->arg2);
                break;
            case _ARG:
                fprintf(file,"ARG %s\n",instrlist->arg1);
                break;
            case _CALL:
                fprintf(file,"%s := CALL %s\n",instrlist->target,instrlist->arg1);
                break;
            case _PARAM:
                fprintf(file,"PARAM %s\n",instrlist->arg1);
                break;
            case _READ:
                fprintf(file,"READ %s\n",instrlist->arg1);
                break;
            case _WRITE:
                fprintf(file,"WRITE %s\n",instrlist->arg1);
                break;
        }
        
        instrlist = instrlist->next;
    }
    
    fclose(file);
    return 0;
}
struct Instr* linkCode(struct Instr* code1,struct Instr* code2){
    if (code1 == NULL) return code2;
    if (code2 == NULL) return code1;
    code1->next = code2;
    return code1;
}
struct Instr* generate_instr(int type,char* arg1,char* arg2,char* target,char* op){
    
    struct Instr* instr = (struct Instr*)malloc(sizeof(struct Instr));
    instr->type = type;
    instr->arg1 = arg1;
    instr->arg2 = arg2;
    instr->target = target;
    strcpy(instr->op,op);
    instr->prev = instr->next = NULL;
    return instr;
}

int get_value(struct treeNode* node){
    //node is INT
    return atoi(node->value);
}

char *lookup(struct Varm* list,char* name){
    while(list!=NULL){
        if (strcmp(list->name,name) == 0){
            return list->vname;
        }
        list = list->next;
    }
    return NULL;
}
struct Instr* translate_Exp(struct treeNode* node,struct Varm* list,char* place){
    //node is Exp
    if (node->sonN == 1 && strcmp(node->sonlist->name,"INT") == 0){
        //Exp -> INT
        int value = get_value(node->sonlist);
        char tmp[20];
        sprintf(tmp,"#%d",value);
        return generate_instr(_ASSIGNOP,tmp,NULL,place,NULL);
    }else if (node->sonN == 1 && strcmp(node->sonlist->name,"ID") == 0){
        //Exp -> ID
        char* var = lookup(list,node->sonlist->value);
        return generate_instr(_ASSIGNOP,var,NULL,place,NULL);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"ASSIGNOP") == 0){
        //Exp -> Exp ASSIGNOP Exp
        
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"PLUS") == 0){
        //Exp -> Exp PLUS Exp
        
    }else if (strcmp(node->sonlist->name,"MINUS") == 0){
        //MINUS Exp
        
    }else if (strcmp(node->sonlist->name,"NOT") == 0 || (node->sonlist->next != NULL && (strcmp(node->sonlist->next->name,"RELOP") == 0 || strcmp(node->sonlist->next->name,"AND") == 0 || strcmp(node->sonlist->next->name,"OR") == 0))){
        //Exp -> Exp RELOP Exp | NOT Exp | Exp AND Exp | Exp OR Exp 
        
    }
    
}

char* translate_VarDec(struct Varm* list,struct treeNode* vardec,struct RTtype* type){
    struct Varm* listlast = list;
    while(listlast->next != NULL){
        listlast = listlast->next;
    }
    if (vardec->sonN == 1){
        //VarDec -> ID
        struct Varm* newvar = malloc(sizeof(struct Varm));
        newvar->type = type->type;
        newvar->name = vardec->sonlist->value;
        newvar->tname = type->tname;
        newvar->vname = newVname();
        newvar->next = newvar->sonlist = NULL;
        listlast->next = newvar;
        return newvar->name;
    }else{
        //VarDec -> VarDec LB INT RB
        struct treeNode* tmp = vardec;
        int deep = 0;
        while(tmp->sonlist != NULL){
            tmp = tmp->sonlist;
            deep++;
        }
        char* name = tmp->value;
        struct Varm* newvar = (struct Varm*)malloc(sizeof(struct Varm));
        newvar->type = type->type + 3;
        newvar->name = name;
        newvar->tname = type->tname;
        newvar->vname = newVname();
        newvar->arraydimension = deep;
        newvar->arraylen = atoi(vardec->sonlist->next->next->value);
        newvar->next = newvar->sonlist = NULL;
        listlast->next = newvar;
        
        struct Varm* newArray = newvar;
        while(vardec->sonlist != NULL){
            newArray->sonlist = (struct Varm*)malloc(sizeof(struct Varm));
            newArray = newArray->sonlist;
            newArray->type = type->type + 3;
            newArray->name = name;
            newArray->tname = type->tname;
            newArray->vname = NULL;
            newArray->arraydimension = --deep;
            newArray->arraylen = atoi(vardec->sonlist->next->next->value);
            newArray->next = newvar->sonlist = NULL;
            vardec = vardec->sonlist;
        }
        
        return newvar->name;
    }
    return NULL;
}
struct Structm* newStruct(){
    struct Structm* rt = (struct Structm*)malloc(sizeof(struct Structm));
    rt->next = NULL;
    rt->feild = (struct Varm*)malloc(sizeof(struct Varm));
    rt->feild->type == 7;
    rt->feild->next = NULL;
    return rt;
}
char* newTemp(){
    tmpN++;
    char* tmp = (char*)malloc(20);
    sprintf(tmp,"t%d",tmpN);
    return tmp;
}
char* newVname(){
    varN++;
    char* tmp = (char*)malloc(20);
    sprintf(tmp,"v%d",varN);
    return tmp;
}
char* newLabel(){
    labelN++;
    char* tmp = (char*)malloc(20);
    sprintf(tmp,"label%d",labelN);
    return tmp;
}
struct RTtype translate_Specifier(struct treeNode* node){
    struct RTtype type;
    if (strcmp(node->sonlist->name,"TYPE") == 0){
        if (strcmp(node->sonlist->value,"int") == 0){
            type.type = _INT;
            return type;
        }else{
            type.type = _FLOAT;
            return type;
        }
        
    }else{
        //node->sonlist is STRUCTSPECIFIER
        if (node->sonlist->sonN == 2){
            //struct define variable
            type.type = _STRUCT;
            type.tname = node->sonlist->sonlist->next->sonlist->value;
            return type;
        }else{
            //struct define
            type.type = _STRUCT;
            struct Structm* newstruct = newStruct();
            struct treeNode* deflist = node->sonlist->sonlist->next->next->next;
            translate_DefList(newstruct->feild,deflist);
            
            if (node->sonlist->sonlist->next->sonN == 1){
                //has a name
                newstruct->name = node->sonlist->sonlist->next->sonlist->value;
            }else{
                //has no name
                newstruct->name = (char*)malloc(30);
                sprintf(newstruct->name,"$_struct_$%d",structN);
                structN++;
            }
           type.tname = newstruct->name;
           if (structlist == NULL) structlist = newstruct;
           else{
                structlist->next = newstruct;
                structlist = newstruct;
           }
           return type;
        }
    }
    type.type = -1;
    return type;
}

struct Instr* translate_Cond(struct treeNode* node,char* label_true,char* label_false,struct Varm* list){
    if (node->sonlist->next!=NULL && strcmp(node->sonlist->next->name,"RELOP") == 0){
        char* t1 = newTemp();
        char* t2 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist,varmlist,t1);
        
    }else if (node->sonlist->next!=NULL && strcmp(node->sonlist->next->name,"Exp") == 0){
        
        
    }else if (node->sonlist->next!=NULL && strcmp(node->sonlist->next->name,"AND") == 0){
        
        
    }else if (node->sonlist->next!=NULL && strcmp(node->sonlist->next->name,"OR") == 0){
        
        
    }else{
        
        
    }
    
    return NULL;
}
struct Instr* translate_Stmt(struct treeNode* node){
    if (node->sonN == 2){
        //Stmt -> Exp SEMI
        return translate_Exp(node->sonlist,varmlist,NULL);
    }else if (node->sonN == 1){
        //Stmt -> CompSt
        return translate_CompSt(node->sonlist);
    }else if (node->sonN == 3){
        //Stmt -> RETURN Exp SEMI
        char* t = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist->next,varmlist,t);
        struct Instr* code2 = generate_instr(_RETURN,t,NULL,NULL,NULL);
        return linkCode(code1,code2);
    }else if (node->sonN == 7){
        //Stmt -> IF LP Exp RP Stmt ELSE Stmt
        char* label1 = newLabel();
        char* label2 = newLabel();
        char* label3 = newLabel();
        struct Instr* code1 = translate_Cond(node->sonlist->next->next,label1,label2,varmlist);
        struct Instr* code2 = translate_Stmt(node->sonlist->next->next->next->next);
        struct Instr* code3 = translate_Stmt(node->sonlist->next->next->next->next->next->next);
        struct Instr* code = linkCode(code1,generate_instr(_LABEL,label1,NULL,NULL,NULL));
        code = linkCode(code,code2);
        code = linkCode(code,generate_instr(_GOTO,label3,NULL,NULL,NULL));
        code = linkCode(code,generate_instr(_LABEL,label2,NULL,NULL,NULL));
        code = linkCode(code,code3);
        return linkCode(code,generate_instr(_LABEL,label3,NULL,NULL,NULL));
    }else if (strcmp(node->sonlist->name,"IF") == 0){
        //Stmt -> IF LP Exp RP Stmt
        char* label1 = newLabel();
        char* label2 = newLabel();
        struct Instr* code1 = translate_Cond(node->sonlist->next->next,label1,label2,varmlist);
        struct Instr* code2 = translate_Stmt(node->sonlist->next->next->next->next);
        struct Instr* code = linkCode(code1,generate_instr(_LABEL,label1,NULL,NULL,NULL));
        code = linkCode(code,code2);
        return linkCode(code,generate_instr(_LABEL,label2,NULL,NULL,NULL));
    }else{
        //Stmt -> WHILE LP Exp RP Stmt
        char* label1 = newLabel();
        char* label2 = newLabel();
        char* label3 = newLabel();
        struct Instr* code1 = translate_Cond(node->sonlist->next->next,label2,label3,varmlist);
        struct Instr* code2 = translate_Stmt(node->sonlist->next->next->next->next);
        struct Instr* code = linkCode(generate_instr(_LABEL,label1,NULL,NULL,NULL),code1);
        code = linkCode(code,generate_instr(_LABEL,label2,NULL,NULL,NULL));
        code = linkCode(code,code2);
        code = linkCode(code,generate_instr(_GOTO,label1,NULL,NULL,NULL));
        return linkCode(code,generate_instr(_LABEL,label3,NULL,NULL,NULL));
    }
    return NULL;
}
struct Instr* translate_StmtList(struct treeNode* node){
    struct Instr* code1 = NULL;
    while(node->sonlist!=NULL){
        struct Instr* code2 = translate_Stmt(node->sonlist);
        code1 = linkCode(code1,code2);
        node = node->sonlist->next;
    }
    return code1;
}
struct Instr* translate_DefList(struct Varm* list ,struct treeNode* node){
    if (node == NULL) return NULL;
    struct treeNode* deflist = node;
    struct Instr* code1 = NULL;
    while(deflist != NULL){
        struct treeNode* def = deflist->sonlist;
        struct RTtype type = translate_Specifier(def->sonlist);
        struct treeNode* declist = def->sonlist->next;
        while(declist != NULL){
            struct treeNode *vardec = declist->sonlist->sonlist;
            //define variable insert to list
            char* name = translate_VarDec(list,vardec,&type);
            char* vname = lookup(list,name);
            if (declist->sonlist->sonN == 3){
                //VarDec ASSIGNOP Exp
                char* t = newTemp();
                struct Instr* code2 = translate_Exp(declist->sonlist->sonlist->next->next,varmlist,t);
                code1 = linkCode(code1,code2);
                code2 = generate_instr(_ASSIGNOP,t,NULL,vname,NULL);
                code1 = linkCode(code1,code2);
                
            }
            declist = declist->sonlist->next->next;
        }
        deflist = deflist->sonlist->next;
    }
    return code1;
}
struct Instr* translate_CompSt(struct treeNode* node){
    if (node == NULL) return NULL;
    struct Instr* code1 = translate_DefList(varmlist,node->sonlist->next);
    struct Instr* code2 = translate_StmtList(node->sonlist->next->next);
    return linkCode(code1,code2);
}

struct Instr* translate_FunDec(struct treeNode* node){
    if (node == NULL) return NULL;
    struct Instr* code1 = generate_instr(_FUNCTION,node->name,NULL,NULL,NULL);
    if (strcmp(node->sonlist->next->next->name,"VarList") == 0){
        struct treeNode* varlist = node->sonlist->next->next;
        while(varlist != NULL){
            struct treeNode* para = varlist->sonlist;
            struct RTtype type = translate_Specifier(para->sonlist);
            char* name = translate_VarDec(varmlist,para->sonlist->next,&type);
            char* vname = lookup(varmlist,name);
            struct Instr* code2 = generate_instr(_PARAM,vname,NULL,NULL,NULL);
            code1 = linkCode(code1,code2);
            if (para->next!=NULL) varlist = para->next->next;
            else varlist = NULL;
        }
    }else{
        //has no varlist
    }
    return code1;
}

struct Instr* translate_ExtDef(struct treeNode* node){
    if (node == NULL) return NULL;
    struct treeNode* son = node->sonlist;
    if (strcmp(node->sonlist->next->name,"SEMI") == 0){
        //Specifier SEMI
        translate_Specifier(node->sonlist);
    }else{
        //函数定义
        struct Instr *code1 = NULL;
        struct Instr *code2 = NULL;
        if (strcmp(son->name,"FunDec") == 0){
            code1 = translate_FunDec(son);
            if (strcmp(son->next->name,"CompSt") == 0){
                code2 = translate_CompSt(son->next);
            }
        }
        return linkCode(code1,code2);
    }
    return NULL;
}

//first level 找到ExtDef进入下一层 
struct Instr* translate(struct treeNode* node){
    if (node == NULL) return NULL;
    struct Instr* code1 = NULL;
    struct Instr* code2 = NULL;
    
    if (strcmp(node->name,"ExtDef") == 0){
        code1 = translate_ExtDef(node);
    }else{
        struct treeNode* son = node->sonlist;
        while(son != NULL){
            code2 = translate(son);
            son = son->next;
        }    
    }
    return linkCode(code1,code2);
}

int middle(struct treeNode* root){
    varmlist = (struct Varm*)malloc(sizeof(struct Varm));
    varmlist->type == 7;
    structlist = NULL;
    
    instrlist = translate(root);
    printInstrList();
    return 0;
}