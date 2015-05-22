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
#define _ArgMax 10
#define _ArgWidth 30

struct Instr* translate_DefList(struct Varm* list ,struct treeNode* node);
char* newVname();
struct Instr* translate_CompSt(struct treeNode* node);
char* newTemp();
char* newLabel();
struct Instr* translate_Cond(struct treeNode* node,char* label_true,char* label_false,struct Varm* list);
struct Instr* translate_Exp(struct treeNode* node,struct Varm* list,char* place);
//-----------------------------------------------------------------------------
int cal_StructLen(char* name){
    struct Structm * list = structlist;
    while(list != NULL){
        if (strcmp(list->name,name) == 0){
            struct Varm* feild = list->feild;
            int space = 0;
            while(feild != NULL){
                space += feild->size;
                feild = feild->next;
            }
            return space;
        }
        list = list->next;
    }
    return -1;
}
int cal_TypeLen(struct RTtype *type){
    if (type->type == _INT || _FLOAT) return 4;
    return cal_StructLen(type->tname);
}

int getExpValue(struct treeNode* node){
    //node is Exp
    if (strcmp(node->sonlist->name,"INT") == 0) return atoi(node->sonlist->value);
    else return -1;
}

int printInstrList(struct Instr* instrlist){
    //FILE* file = fopen("output.rs","a+");
    FILE* file = fopen("output.ir","w+");
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
    //fprintf(file,"---------------------------------------\n");
    fclose(file);
    return 0;
}
struct Instr* linkCode(struct Instr* code1,struct Instr* code2){
    if (code1 == NULL) return code2;
    if (code2 == NULL) return code1;
    struct Instr* code1last = code1;
    while(code1last->next != NULL) code1last = code1last->next;
    code1last->next = code2;
    return code1;
}
struct Instr* generate_instr(int type,char* arg1,char* arg2,char* target,char* op){
    
    struct Instr* instr = (struct Instr*)malloc(sizeof(struct Instr));
    instr->type = type;
    instr->arg1 = arg1;
    instr->arg2 = arg2;
    instr->target = target;
    if (op == NULL) instr->op = NULL; else{
        instr->op = op;
        //strcpy(instr->op,op);
    }
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
struct Instr* translate_Args(struct treeNode* node,struct Varm* list,char* arglist,int* argn){
    if (node->sonN == 1){
        //Args -> Exp
        char* t1 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist,list,t1);
        strcpy(&arglist[*argn * _ArgWidth],t1);
        *argn++;
        return code1;
    }else{
        //Args -> Exp COMMA Args
        char* t1 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist,list,t1);
        strcpy(&arglist[*argn * _ArgWidth],t1);
        struct Instr* code2 = translate_Args(node->sonlist->next->next,list,arglist,argn);
        return linkCode(code1,code2);
    }
    
}

struct Instr* cal_offset(struct treeNode* node,char *offset){
    //Exp -> Exp LB Exp RB
    struct Instr* code1 = NULL;
    
    struct Varm* var = varmlist;
    struct treeNode* id = node;
    while(id!=NULL){
        if (strcmp(id->name,"ID") == 0) break;
        id = id->sonlist;
    }
    while(var != NULL){
        if (strcmp(var->name,id->value) == 0){
            struct Instr* code2 = generate_instr(_ASSIGNOP,var->vname,NULL,offset,NULL);
            code1 = linkCode(code1,code2);
            break;
        }
        var = var->next;    
    }
    int size = 0;
    if (var == NULL) printf("can not find this var.\n");
    if (var->type == _STRUCT || var->type == _STRUCTARRAY){
        size = cal_StructLen(var->tname);
    }else size = 4;
    
    struct treeNode* exp = node;
    while(exp->sonN == 4){
        char* t = newTemp();
        struct Instr* code2 = translate_Exp(exp->sonlist->next->next,varmlist,t);
        char* sizes = (char*)malloc(20);
        sprintf(sizes,"#%d",size);
        struct Instr* code3 = generate_instr(_STAR,t,sizes,t,NULL);
        struct Instr* code4 = generate_instr(_ADD,offset,t,offset,NULL);
        code1 = linkCode(code1,linkCode(code2,linkCode(code3,code4)));
        exp = exp->sonlist;
        size = size * var->arraylen;
        var = var->sonlist;
    }
    
    return code1;
    
    
    //return code1;
    
}

struct Instr* translate_Exp(struct treeNode* node,struct Varm* list,char* place){
    //node is Exp
    if (node->sonN == 1 && strcmp(node->sonlist->name,"INT") == 0){
        //Exp -> INT
        int value = get_value(node->sonlist);
        char* tmp = (char*)malloc(20);
        sprintf(tmp,"#%d",value);
        return generate_instr(_ASSIGNOP,tmp,NULL,place,NULL);
    }else if (node->sonN == 1 && strcmp(node->sonlist->name,"ID") == 0){
        //Exp -> ID
        char* vname = lookup(list,node->sonlist->value);
        return generate_instr(_ASSIGNOP,vname,NULL,place,NULL);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"ASSIGNOP") == 0){
        //Exp -> Exp ASSIGNOP Exp
        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        //attention , Exp1 can be not ID.
        //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        char* vname = NULL;
        struct Instr* code0 = NULL;
        if (node->sonlist->sonN == 1)
            vname = lookup(list,node->sonlist->sonlist->value);
        else{
            vname = newTemp();
            code0 = translate_Exp(node->sonlist,NULL,vname);
        }
        char* t = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist->next->next,list,t);
        struct Instr* code2 = NULL;
        if (code0 != NULL){
            code1 = linkCode(code0,code1);
            code2 = generate_instr(_ASSIGNOP_GETVALUE,t,NULL,vname,NULL);
        }else{ 
            code2 = generate_instr(_ASSIGNOP,t,NULL,vname,NULL);
        }
        struct Instr* code = linkCode(code1,code2);
        if (place == NULL) return code; 
        else return linkCode(code,generate_instr(_ASSIGNOP,vname,NULL,place,NULL));
        
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"PLUS") == 0){
        //Exp -> Exp PLUS Exp
        char* t1 = newTemp();
        char* t2 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist,list,t1);
        struct Instr* code2 = translate_Exp(node->sonlist->next->next,list,t2);
        struct Instr* code3 = generate_instr(_ADD,t1,t2,place,NULL);
        return linkCode(linkCode(code1,code2),code3);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"MINUS") == 0){
        //Exp -> Exp MINUS Exp
        char* t1 = newTemp();
        char* t2 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist,list,t1);
        struct Instr* code2 = translate_Exp(node->sonlist->next->next,list,t2);
        struct Instr* code3 = generate_instr(_MINUS,t1,t2,place,NULL);
        return linkCode(linkCode(code1,code2),code3);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"STAR") == 0){
        //Exp -> Exp STAR Exp
        char* t1 = newTemp();
        char* t2 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist,list,t1);
        struct Instr* code2 = translate_Exp(node->sonlist->next->next,list,t2);
        struct Instr* code3 = generate_instr(_STAR,t1,t2,place,NULL);
        return linkCode(linkCode(code1,code2),code3);
    }else if (node->sonlist->next != NULL && strcmp(node->sonlist->next->name,"DIV") == 0){
        //Exp -> Exp DIV Exp
        char* t1 = newTemp();
        char* t2 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist,list,t1);
        struct Instr* code2 = translate_Exp(node->sonlist->next->next,list,t2);
        struct Instr* code3 = generate_instr(_DIV,t1,t2,place,NULL);
        return linkCode(linkCode(code1,code2),code3);
    }else if (strcmp(node->sonlist->name,"LP") == 0){
        //Exp -> LP Exp RP
        char* t = newTemp();
        return translate_Exp(node->sonlist->next,list,t);
    }else if (strcmp(node->sonlist->name,"MINUS") == 0){
        //MINUS Exp
        char* t1 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist->next,list,t1);
        struct Instr* code2 = generate_instr(_MINUS,"#0",t1,place,NULL);
        return linkCode(code1,code2);
    }else if (strcmp(node->sonlist->name,"NOT") == 0 || (node->sonlist->next != NULL && (strcmp(node->sonlist->next->name,"RELOP") == 0 || strcmp(node->sonlist->next->name,"AND") == 0 || strcmp(node->sonlist->next->name,"OR") == 0))){
        //Exp -> Exp RELOP Exp | NOT Exp | Exp AND Exp | Exp OR Exp 
        char* label1 = newLabel();
        char* label2 = newLabel();
        struct Instr* code1 = generate_instr(_ASSIGNOP,"#0",NULL,place,NULL);
        struct Instr* code2 = translate_Cond(node,label1,label2,list);
        struct Instr* code3 = generate_instr(_LABEL,label1,NULL,NULL,NULL);
        code3 = linkCode(code3,generate_instr(_ASSIGNOP,"#1",NULL,place,NULL));
        return linkCode(linkCode(code1,code2),code3);
    }else if (node->sonlist->next->next!=NULL && strcmp(node->sonlist->next->next->name,"RP") == 0){
        //Exp -> ID LP RP
        if (strcmp(node->sonlist->value,"read") == 0){
            return generate_instr(_READ,place,NULL,NULL,NULL);
        }else{
            return generate_instr(_CALL,node->sonlist->value,NULL,place,NULL);
        }
    }else if (node->sonlist->next->next!=NULL && strcmp(node->sonlist->next->next->name,"Args") == 0){
        //Exp -> ID LP Args RP
        char* arglist = (char*)malloc(_ArgMax * _ArgWidth);
        int argn = 0;
        struct Instr* code1 = translate_Args(node->sonlist->next->next,list,arglist,&argn);
        if (strcmp(node->sonlist->value,"write") == 0) return linkCode(code1,generate_instr(_WRITE,&arglist[0],NULL,NULL,NULL));
        int i = 0;
        struct Instr* code2 = NULL;
        for (i = 0; i<argn; i++) code2 = linkCode(code2,generate_instr(_ARG,&arglist[i * _ArgWidth],NULL,NULL,NULL));
        return linkCode(linkCode(code1,code2),generate_instr(_CALL,node->sonlist->value,NULL,place,NULL));
    }else if (node->sonN == 4 && strcmp(node->sonlist->next->name,"LB") == 0){
        //Exp -> Exp LB Exp RB
        //char* offset = newTemp();
        if (list != NULL){
            struct Instr* code1 = cal_offset(node,place);
            struct Instr* code2 = generate_instr(_GETVALUE,place,NULL,place,NULL);
            return linkCode(code1,code2);
        }else{
            struct Instr* code1 = cal_offset(node,place);
            return code1;
        }
        /*if (strcmp(node->sonlist->name,"ID") == 0){
            
        }else{
            char* t1 = newTemp();
            char* t2 = newTemp();
            struct Instr* code1 = translate(node->sonlist,list,t1);

        }*/
    }else if (node->sonN == 3 && strcmp(node->sonlist->next->name,"DOT") == 0){
        //Exp -> Exp DOT ID
        struct treeNode* id = node->sonlist;
        while(id != NULL){
            if (strcmp(id->name,"ID") == 0) break;
            id = id->sonlist;
        }
        
        struct Instr* code1 = translate_Exp(node->sonlist,NULL,place);
        
        return NULL;
    }else{
        return NULL;
    }
    
}

struct Instr* translate_VarDec(int flag,struct Varm* list,struct treeNode* vardec,struct RTtype* type,char **rtname){
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
        newvar->size = cal_TypeLen(type);
        newvar->next = newvar->sonlist = NULL;
        listlast->next = newvar;
        *rtname = newvar->name;
        return NULL;
    }else{
        //VarDec -> VarDec LB INT RB
        struct treeNode* tmp = vardec;
        int deep = 0;
        while(tmp->sonlist != NULL){
            tmp = tmp->sonlist;
            deep++;
        }
        char* name = tmp->value;
        int space = 1;
        struct Varm* newvar = (struct Varm*)malloc(sizeof(struct Varm));
        newvar->type = type->type + 3;
        newvar->name = name;
        newvar->tname = type->tname;
        newvar->vname = newVname();
        newvar->arraydimension = deep;
        newvar->arraylen = atoi(vardec->sonlist->next->next->value);
        newvar->next = newvar->sonlist = NULL;
        listlast->next = newvar;
        space *= newvar->arraylen;
        struct Varm* newArray = newvar;
        vardec = vardec->sonlist;
        while(vardec->sonlist != NULL && strcmp(vardec->sonlist->name,"VarDec") == 0){
            newArray->sonlist = (struct Varm*)malloc(sizeof(struct Varm));
            newArray = newArray->sonlist;
            newArray->type = type->type + 3;
            newArray->name = name;
            newArray->tname = type->tname;
            newArray->vname = NULL;
            newArray->arraydimension = --deep;
            newArray->arraylen = atoi(vardec->sonlist->next->next->value);
            space *= newArray->arraylen;
            newArray->next = newArray->sonlist = NULL;
            vardec = vardec->sonlist;
        }

        //space * checktype;
   
        newvar->size = space * cal_TypeLen(type);
   
        newArray = newvar;
        while(newArray->sonlist != NULL){
            newArray->sonlist->size = newArray->size / newArray->arraylen;
            newArray = newArray->sonlist;
        }
        *rtname = newvar->name;
        if (list == varmlist && flag == 0){
            char* size = (char *)malloc(20);
            sprintf(size,"%d",newvar->size);
            return generate_instr(_DEC,newvar->vname,size,NULL,NULL);
        }else return NULL;
    }
    *rtname = NULL;
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
char* get_relop(struct treeNode* node){
    //node is relop
    return node->value;
}
struct Instr* translate_Cond(struct treeNode* node,char* label_true,char* label_false,struct Varm* list){
    if (node->sonlist->next!=NULL && strcmp(node->sonlist->next->name,"RELOP") == 0){
        char* t1 = newTemp();
        char* t2 = newTemp();
        struct Instr* code1 = translate_Exp(node->sonlist,list,t1);
        struct Instr* code2 = translate_Exp(node->sonlist->next->next,list,t2);
        char* op = get_relop(node->sonlist->next);
        struct Instr* code3 = generate_instr(_IF,t1,t2,label_true,op);
        struct Instr* code = linkCode(code1,code2);
        code = linkCode(code,code3);
        return linkCode(code,generate_instr(_GOTO,label_false,NULL,NULL,NULL));
    }else if (node->sonlist->next!=NULL && strcmp(node->sonlist->next->name,"Exp") == 0){
        //NOT Exp
        return translate_Cond(node->sonlist->next,label_false,label_true,list);
    }else if (node->sonlist->next!=NULL && strcmp(node->sonlist->next->name,"AND") == 0){
        char* label1 = newLabel();
        struct Instr* code1 = translate_Cond(node->sonlist,label1,label_false,list);
        struct Instr* code2 = translate_Cond(node->sonlist->next->next,label_true,label_false,list);
        struct Instr* code = linkCode(code1,generate_instr(_LABEL,label1,NULL,NULL,NULL));
        return linkCode(code,code2);
    }else if (node->sonlist->next!=NULL && strcmp(node->sonlist->next->name,"OR") == 0){
        char* label1 = newLabel();
        struct Instr* code1 = translate_Cond(node->sonlist,label_true,label1,list);
        struct Instr* code2 = translate_Cond(node->sonlist->next->next,label_true,label_false,list);
        struct Instr* code = linkCode(code1,generate_instr(_LABEL,label1,NULL,NULL,NULL));
        return linkCode(code,code2);
    }else{
        char* t = newTemp();
        struct Instr* code1 = translate_Exp(node,list,t);
        struct Instr* code2 = generate_instr(_IF,t,"#0",label_true,"!=");
        struct Instr* code = linkCode(code1,code2);
        return linkCode(code,generate_instr(_GOTO,label_false,NULL,NULL,NULL));
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
        code = linkCode(code,generate_instr(_LABEL,label3,NULL,NULL,NULL));
        return code;
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
    //printf("%d - %s\n",node->lineno,node->name);
    struct treeNode* deflist = node;
    struct Instr* code1 = NULL;
    while(deflist != NULL){
        struct treeNode* def = deflist->sonlist;
        if (def == NULL) break;
        struct RTtype type = translate_Specifier(def->sonlist);
        struct treeNode* declist = def->sonlist->next;
        
        while(declist != NULL){
            struct treeNode *vardec = declist->sonlist->sonlist;
            //define variable insert to list
            char* name = NULL;
            struct Instr* codetype = translate_VarDec(0,list,vardec,&type,&name);
            if (list == varmlist){
                //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                //注意这里，需要对struct 进行DEC空间
                //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
                code1 = linkCode(code1,codetype);
                char* vname = lookup(list,name);
                if (declist->sonlist->sonN == 3){
                    //VarDec ASSIGNOP Exp
                    char* t = newTemp();
                    struct Instr* code2 = translate_Exp(declist->sonlist->sonlist->next->next,varmlist,t);
                    code1 = linkCode(code1,code2);
                    code2 = generate_instr(_ASSIGNOP,t,NULL,vname,NULL);
                    code1 = linkCode(code1,code2);
                
                }
            }else{
                //struct feild
                
            }
            if (declist->sonN == 1) declist = NULL; 
            else declist = declist->sonlist->next->next;
        }
        if (deflist->sonlist == NULL) deflist = NULL; else deflist = deflist->sonlist->next;
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
    struct Instr* code1 = generate_instr(_FUNCTION,node->sonlist->value,NULL,NULL,NULL);
    if (strcmp(node->sonlist->next->next->name,"VarList") == 0){
        struct treeNode* varlist = node->sonlist->next->next;
        while(varlist != NULL){
            struct treeNode* para = varlist->sonlist;
            struct RTtype type = translate_Specifier(para->sonlist);
            char* name = NULL;
            struct Instr* codetype = translate_VarDec(1,varmlist,para->sonlist->next,&type,&name);
            code1 = linkCode(code1,codetype);
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
    struct treeNode* son = node->sonlist->next;
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
    //node is ExtDefList
    if (node == NULL) return NULL;
    struct Instr* code1 = NULL;
    struct Instr* code2 = NULL;
    
    if (node->sonlist == NULL) return NULL;
    
    if (strcmp(node->sonlist->name,"ExtDef") == 0){
        code1 = translate_ExtDef(node->sonlist);
    }
    code2 = translate(node->sonlist->next);
    
    return linkCode(code1,code2);
}

int middle(struct treeNode* root){
    varmlist = (struct Varm*)malloc(sizeof(struct Varm));
    varmlist->type == 7;
    varmlist->name = (char*)malloc(30);
    sprintf(varmlist->name,"$_start_");
    structlist = NULL;
    
    instrlist = translate(root->sonlist);
    printf("strat printInstrList...\n");
    printInstrList(instrlist);
    return 0;
}