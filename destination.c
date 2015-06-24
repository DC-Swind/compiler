#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "common.h"

#define __LABEL 1
#define __LI 2
#define __MOVE 3
#define __ADDI 4
#define __ADD 5
#define __SUB 6
#define __MUL 7
#define __DIV 8
#define __MFLO 9
#define __LW 10
#define __SW 11
#define __J 12
#define __JAL 13
#define __JR 14
#define __BEQ 15
#define __BNE 16
#define __BGT 17
#define __BLT 18
#define __BGE 19
#define __BLE 20
#define __LA 21

#define __FUNC 30
#define __SYSCALL 31

#define __DATA 101
#define __ASCIIZ 102
#define __GLOBL 103
#define __TEXT 104

#define __UNDEFINED 200
struct Code{
    int type;
    int reg1;
    int reg2;
    int reg3;
    char* arg1;
    struct Code* next;
};
struct Data{
    int type;
    char* name;
    char* value;
    struct Data* next;
};
struct Code *codelist = NULL;
struct Data *datalist = NULL;
int allocatetime = 0;
int argna = 0;
int argnb = 0;
struct VarReg{
    char* var;  //v1 or t1
    int reg;    //reg
    int stackoffset;
    int time;
    struct VarReg* next;
};
struct VarReg* varreg = NULL;
int regf[32];
struct Code* newCode(int type,int reg1,int reg2,int reg3,char* arg1);
struct Code* linkcode(struct Code* code1, struct Code* code2);
int printVarReg(){
    struct VarReg* p = varreg;
    while(p != NULL){
        printf("var:%s - reg:%d - offset:%d\n",p->var,p->reg,p->stackoffset);
        p = p->next;
    }
    printf("-------------------------------------\n");
    return 0;
}
int clearReg(){
    struct VarReg* p = varreg;
    while(p != NULL){
        p->reg = -1;
        p = p->next;
    }
    int i;
    for(i=8; i<23; i++) regf[i] = 0;
    return 0;
}
int saveAllReg(){
    
    struct VarReg* p = varreg;
    while(p != NULL){
        if (p->reg != -1){
            if (p->var[0] == '#') {
                //printf("saveAllReg occured error.p->var is %s\n",p->var);
                regf[p->reg] = 0;
                p->reg = -1;
                p = p->next;
                continue;
            }
            struct Code* code = NULL;
            int reg = convertreg("$fp");
            char arg[20];
            sprintf(arg,"-%d",p->stackoffset);
            code = newCode(__ADDI,24,reg,-1,arg);
            linkcode(codelist,code);
            code = newCode(__SW,p->reg,24,-1,NULL);
            linkcode(codelist,code);
            regf[p->reg] = 0;
            p->reg = -1;
        }
        p = p->next;
    }
    
    return 0;
}

int getReg(char* varname,int target){
    //保留24/25两个寄存器
    //first , check varreg list, 如果有这个varname，并且分配了reg，则立即返回
    struct VarReg* p = varreg;
    struct VarReg* tail = NULL;
    while(p != NULL){
        tail = p;
        p = p->next;
    }
    p = varreg;
    while(p != NULL){
        if (strcmp(p->var,varname) == 0) break;
        p = p->next;
    }
    if(p != NULL && p->reg != -1) return p->reg;

    
    
    if (varname[0] == '#'){
        struct VarReg* newvar = malloc(sizeof(struct VarReg));
        newvar->var = malloc(20);
        strcpy(newvar->var,varname);
        newvar->reg = -1;
        newvar->time = allocatetime++;
        newvar->next = NULL;
        
        int i,mintime = 999999999;
        struct VarReg* min = NULL;
        struct VarReg* minpre = NULL;
        for(i = 8; i<23; i++) if (regf[i] == 0){
            newvar->reg = i;
            if(varreg == NULL) varreg = newvar;
            else tail->next = newvar;
            
            struct Code* code = newCode(__LI,i,-1,-1,&varname[1]);
            linkcode(codelist,code);
            regf[i] = 1;
            return i;
        }
        p = varreg;
        while(p != NULL){
            if (p->reg != -1 && p->time < mintime){
                mintime = p->time;
                min = p;
            }
            p = p->next;
        }
        newvar->reg = min->reg;
        tail->next = newvar;   //不可能为NULL，因为寄存器都分配完了，必然不会NULL
        if(min->var[0] == '#'){
            minpre = varreg;
            while(minpre->next != min) minpre = minpre->next;
            minpre->next = min->next;
            free(min);
        }else{
            struct Code* code = NULL;
            int reg = convertreg("$fp");
            char arg[20];
            sprintf(arg,"-%d",min->stackoffset);
            code = newCode(__ADDI,24,reg,-1,arg);
            linkcode(codelist,code);
            code = newCode(__SW,min->reg,24,-1,NULL);
            linkcode(codelist,code);
        }
        struct Code* code = newCode(__LI,newvar->reg,-1,-1,&varname[1]);
        linkcode(codelist,code);
        min->reg = -1;
        return newvar->reg;
    }
    
    if (p == NULL){
        printf("getReg occured error: p is null\n");
    }
    p->time = allocatetime++;

    int i;
    for(i = 8; i<23; i++) if (regf[i] == 0){
        p->reg = i;
        regf[i] = 1;
        
        if (target == 0){
            struct Code* code = NULL;
            int reg = convertreg("$fp");
            char arg[20];
            sprintf(arg,"-%d",p->stackoffset);
            code = newCode(__ADDI,24,reg,-1,arg);
            linkcode(codelist,code);
            code = newCode(__LW,p->reg,24,-1,NULL);
            linkcode(codelist,code);
        }
    
        return p->reg;
    }

    
    int mintime = 999999999;
    struct VarReg *min = NULL;
    struct VarReg *minpre = NULL;
    struct VarReg *pp = varreg;
    
    while(pp != NULL){
        if (pp->reg != -1 && pp->time < mintime){
            mintime = pp->time;
            min = pp;
        }
        pp = pp->next;
    }

    //printf("old:%s,%d   new:%s,%d    target:%d\n",min->var,min->reg,p->var,p->reg,target);
    p->reg = min->reg;

    if(min->var[0] == '#'){
        minpre = varreg;
        while(minpre->next != min) minpre = minpre->next;
        assert(minpre != NULL);
        minpre->next = min->next;
        free(min);
    }else{
        struct Code* code = NULL;
        int reg = convertreg("$fp");
        char arg[20];
        sprintf(arg,"-%d",min->stackoffset);
        code = newCode(__ADDI,24,reg,-1,arg);
        linkcode(codelist,code);
        code = newCode(__SW,min->reg,24,-1,NULL);
        linkcode(codelist,code);
    }
    min->reg = -1;
    if (target == 0){
        //printf();
        
        struct Code* code = NULL;
        int reg = convertreg("$fp");
        char arg[20];
        sprintf(arg,"-%d",p->stackoffset);
        code = newCode(__ADDI,24,reg,-1,arg);
        linkcode(codelist,code);
        code = newCode(__LW,p->reg,24,-1,NULL);
        linkcode(codelist,code);
        
    }
    
    
    return p->reg;    
}
int convertreg(char* name){
    if (strcmp(name,"$zero") == 0) return 0;
    if (strcmp(name,"$at") == 0) return 1;
    if (strcmp(name,"$v0") == 0) return 2;
    if (strcmp(name,"$gp") == 0) return 28;
    if (strcmp(name,"$sp") == 0) return 29;
    if (strcmp(name,"$fp") == 0) return 30;
    if (strcmp(name,"$ra") == 0) return 31;
    if (strcmp(name,"$a0") == 0) return 4;
    if (strcmp(name,"$a1") == 0) return 5;
    if (strcmp(name,"$a2") == 0) return 6;
    if (strcmp(name,"$a3") == 0) return 7;
    printf("convert: %s\n",name);
    return -1;
}
char* regToString(int i){
    
    return NULL;
}
struct Code *linkcode(struct Code* code1,struct Code* code2){
    while(code1->next!=NULL) code1 = code1->next;
    code1->next = code2;
    return code1;
}
struct Data *linkdata(struct Data* data1,struct Data* data2){
    while(data1->next != NULL) data1 = data1->next;
    data1->next = data2;
    return data1;
}
struct Code *newCode(int type,int reg1,int reg2,int reg3,char* arg1){
    struct Code* code = malloc(sizeof(struct Code));
    code->type = type;
    code->reg1 = reg1;
    code->reg2 = reg2;
    code->reg3 = reg3;
    if (arg1 == NULL) code->arg1 = NULL; 
    else{
        code->arg1 = malloc(20);
        
        sprintf(code->arg1,"%s",arg1);
    }
    code->next = NULL;
    return code;
}
struct Data *newData(int type,char* name, char* value){
    struct Data* data = malloc(sizeof(struct Data));
    data->type = type;
    if (name == NULL) data->name = NULL;
    else{
        data->name = malloc(20);
        sprintf(data->name,"%s",name);
    }
    if (value == NULL) data->value = NULL;
    else{
        data->value = malloc(20);
        sprintf(data->value,"%s",value);
    }
    data->next = NULL;
    return data;
}
int start(){
    struct Instr *p = instrlist;
    while(p!=NULL){
        switch(p->type){
            case _LABEL:{
                saveAllReg();
                struct Code* code = newCode(__LABEL,-1,-1,-1,p->arg1);
                linkcode(codelist,code);
                break;
            }
            case _FUNCTION:{
                //printf("func:%s\n",p->arg1);
                argnb = 0;
                struct Code* code = newCode(__FUNC,-1,-1,-1,p->arg1);
                linkcode(codelist,code);
                int reg1 = convertreg("$fp");
                int reg2 = convertreg("$sp");
                code = newCode(__MOVE,reg1,reg2,-1,NULL);
                linkcode(codelist,code);
                
                int offset = 4;
                struct Instr *pp = p->next;
                //遇到RETURN STOP,ERROr,不能到return stop，如果多个return会出问题，所以到func结束
                while(pp!=NULL && pp->type!=_FUNCTION){
                    if(pp->type==_ASSIGNOP||pp->type==_ADD||pp->type==_MINUS||pp->type==_STAR||pp->type==_DIV||pp->type==_GETVALUE||pp->type==_GETADDR||pp->type==_CALL){
                        struct VarReg* var = varreg;
                        struct VarReg* newvar = malloc(sizeof(struct VarReg));
                        newvar->var = malloc(20);
                        strcpy(newvar->var,pp->target);
                        newvar->reg = -1;
                        newvar->stackoffset = offset;
                        newvar->next = NULL;
                        
                        int flag = 1;
                        if(varreg == NULL){
                            varreg = newvar;
                            offset += 4;
                        }else{
                            while(var->next != NULL){
                                if(strcmp(var->var,newvar->var) == 0){
                                    flag = 0;
                                    break;
                                }
                                var = var->next;
                            }
                            if (flag && strcmp(var->var,newvar->var) != 0){
                                var->next = newvar;
                                offset += 4;
                            }
                        }
                    }else if (pp->type == _DEC || pp->type == _READ){
                        struct VarReg* var = varreg;
                        struct VarReg* newvar = malloc(sizeof(struct VarReg));
                        newvar->var = malloc(20);
                        strcpy(newvar->var,pp->arg1);
                        newvar->reg = -1;
                        newvar->stackoffset = offset;
                        newvar->next = NULL;
                        
                        int flag = 1;
                        if(varreg == NULL){
                            varreg = newvar;
                            offset += 4;
                        }else{
                            while(var->next != NULL){
                                if(strcmp(var->var,newvar->var) == 0){
                                    flag = 0;
                                    break;
                                }
                                var = var->next;
                            }
                            if (flag && strcmp(var->var,newvar->var) != 0){
                                var->next = newvar;
                                offset += 4;
                            }
                        }
                    }else if (pp->type == _PARAM){
                        argna++;
                        argnb++;
                        struct VarReg* var = varreg;
                        struct VarReg* newvar = malloc(sizeof(struct VarReg));
                        newvar->var = malloc(20);
                        strcpy(newvar->var,pp->arg1);
                        newvar->reg = -1;
                        newvar->stackoffset = offset;
                        newvar->next = NULL;
                        
                        int flag = 1;
                        if(varreg == NULL){
                            varreg = newvar;
                            offset += 4;
                        }else{
                            while(var->next != NULL){
                                if(strcmp(var->var,newvar->var) == 0){
                                    flag = 0;
                                    break;
                                }
                                var = var->next;
                            }
                            if (flag && strcmp(var->var,newvar->var) != 0){
                                var->next = newvar;
                                offset += 4;
                            }
                        }
                    }
                    pp = pp->next;
                }
                char arg[20];
                sprintf(arg,"-%d",offset-4);
                code = newCode(__ADDI,reg2,reg2,-1,arg);
                linkcode(codelist,code);
                //printVarReg();
                //printf("func end\n");
                break;
            }
            case _ASSIGNOP:{
                if(p->arg1[0] != '#'){
                    int reg2 = getReg(p->arg1,0);
                    int reg1 = getReg(p->target,1);
                    struct Code* code = newCode(__MOVE,reg1,reg2,-1,NULL);
                    linkcode(codelist,code);
                }else{
                    int reg1 = getReg(p->target,1);
                    struct Code* code = newCode(__LI,reg1,-1,-1,&p->arg1[1]);
                    linkcode(codelist,code);
                }
                break;
            }
            case _ADD:{
                int reg1 = getReg(p->arg1,0);
                if (p->arg2[0] != '#'){
                    int reg2 = getReg(p->arg2,0);
                    int reg0 = getReg(p->target,1);
                    struct Code* code = newCode(__ADD,reg0,reg1,reg2,NULL);
                    linkcode(codelist,code);
                }else{
                    int reg0 = getReg(p->target,1);
                    struct Code* code = newCode(__ADDI,reg0,reg1,-1,&p->arg2[1]);
                    linkcode(codelist,code);
                }
                break;
            }
            case _MINUS:{
                int reg1 = getReg(p->arg1,0);
                if (p->arg2[0] != '#'){
                    int reg2 = getReg(p->arg2,0);
                    int reg0 = getReg(p->target,1);
                    struct Code* code = newCode(__SUB,reg0,reg1,reg2,NULL);
                    linkcode(codelist,code);
                }else{
                    int reg0 = getReg(p->target,1);
                    char arg[20];
                    sprintf(arg,"-%s",&p->arg2[1]);
                    struct Code* code = newCode(__ADDI,reg0,reg1,-1,arg);
                    linkcode(codelist,code);
                }
                break;
            }
            case _STAR:{
                int reg1 = getReg(p->arg1,0);
                int reg2 = getReg(p->arg2,0);
                int reg0 = getReg(p->target,1);
                struct Code* code = newCode(__MUL,reg0,reg1,reg2,NULL);
                linkcode(codelist,code);
                break;
            }
            case _DIV:{
                int reg1 = getReg(p->arg1,0);
                int reg2 = getReg(p->arg2,0);
                int reg0 = getReg(p->target,1);
                struct Code* code1 = newCode(__DIV,reg1,reg2,-1,NULL);
                struct Code* code2 = newCode(__MFLO,reg0,-1,-1,NULL);
                linkcode(codelist,linkcode(code1,code2));
                break;
            }
            case _GETADDR:{
                int reg2 = getReg(p->arg1,0);
                int reg1 = getReg(p->target,1);
                struct Code* code = newCode(__MOVE,reg1,reg2,-1,NULL);
                linkcode(codelist,code);
                //fprintf(file,"%s := &%s\n",instrlist->target,instrlist->arg1);
                break;
            }
            case _GETVALUE:{
                int reg2 = getReg(p->arg1,0);
                int reg1 = getReg(p->target,1);
                struct Code* code = newCode(__LW,reg1,reg2,-1,NULL);
                linkcode(codelist,code);
                break;
            }
            case _ASSIGNOP_GETVALUE:{
                int reg1 = getReg(p->arg1,0);
                int reg2 = getReg(p->target,1);
                struct Code* code = newCode(__SW,reg1,reg2,-1,NULL);
                linkcode(codelist,code);
                break;
            }
            case _GOTO:{
                saveAllReg();
                struct Code* code = newCode(__J,-1,-1,-1,p->arg1);
                linkcode(codelist,code);
                break;
            }
            case _IF:{
                struct Code* code = NULL;
                int reg1 = getReg(p->arg1,0);
                int reg2 = getReg(p->arg2,0);
                //printVarReg();
                saveAllReg();
                if (strcmp(p->op,"==") == 0){
                    code = newCode(__BEQ,reg1,reg2,-1,p->target);
                }else if (strcmp(p->op,"!=") == 0){
                    code = newCode(__BNE,reg1,reg2,-1,p->target);
                }else if (strcmp(p->op,">") == 0){
                    code = newCode(__BGT,reg1,reg2,-1,p->target);
                }else if (strcmp(p->op,"<") == 0){
                    code = newCode(__BLT,reg1,reg2,-1,p->target);
                }else if (strcmp(p->op,">=") == 0){
                    code = newCode(__BGE,reg1,reg2,-1,p->target);
                }else if (strcmp(p->op,"<=") == 0){
                    code = newCode(__BLE,reg1,reg2,-1,p->target);
                }
                linkcode(codelist,code);
                //printf("IF\n");
                break;
            }
            case _RETURN:{
                int reg2 = convertreg("$v0");
                int reg3 = convertreg("$ra");
                struct Code* code1 = NULL;
                if (p->arg1[0] == '#'){
                    code1 = newCode(__LI,reg2,-1,-1,&p->arg1[1]);
                }else{
                    int reg1 = getReg(p->arg1,0);
                    code1 = newCode(__MOVE,reg2,reg1,-1,NULL);
                }
                //move $fp to sp
                int reg01 = convertreg("$fp");
                int reg02 = convertreg("$sp");
                struct Code* code = newCode(__MOVE,reg02,reg01,-1,NULL);
                linkcode(codelist,code);
                
                struct Code* code2 = newCode(__JR,reg3,-1,-1,NULL);
                linkcode(codelist,linkcode(code1,code2));
                clearReg();
                break;
            }
            case _DEC:{
                struct Code* code = NULL;
                int reg1 = convertreg("$sp");
                char size[20];
                
                sprintf(size,"-%d",atoi(p->arg2));
                code = newCode(__ADDI,reg1,reg1,-1,size);
                linkcode(codelist,code);
                int reg2 = getReg(p->arg1,0);
                code = newCode(__MOVE,reg2,reg1,-1,NULL);
                linkcode(codelist,code);
                //fprintf(file,"DEC %s %s\n",instrlist->arg1,instrlist->arg2);
                break;
            }
            case _ARG:{
               
                /*
                struct Code* code = newCode(__UNDEFINED,-1,-1,-1,NULL);
                linkcode(codelist,code);
                break;
                */
                //第一个ARG存$ra,$fp
                if (argna == 0){
                    struct Code* code0 = NULL;
                    int reg01 = convertreg("$sp");
                    int reg02 = convertreg("$ra");
                    int reg03 = convertreg("$fp");
                    code0 = newCode(__ADDI,reg01,reg01,-1,"-4");
                    linkcode(codelist,code0);
                    code0 = newCode(__SW,reg02,reg01,-1,NULL);
                    linkcode(codelist,code0);
                    code0 = newCode(__ADDI,reg01,reg01,-1,"-4");
                    linkcode(codelist,code0);
                    code0 = newCode(__SW,reg03,reg01,-1,NULL);
                    linkcode(codelist,code0);
                }

                int reg1 = getReg(p->arg1,0);

                if (argna <4){
                    char argreg[20];
                    sprintf(argreg,"$a%d",argna);
                    int reg2 = convertreg(argreg);
                    struct Code* code1 = newCode(__MOVE,reg2,reg1,-1,NULL);
                    linkcode(codelist,code1);
                }else{
                    int reg2 = convertreg("$sp");
                    
                    struct Code* code = NULL;
                    code = newCode(__ADDI,reg2,reg2,-1,"-4");
                    linkcode(codelist,code);
                    code = newCode(__SW,reg1,reg2,-1,NULL);
                    linkcode(codelist,code);
                    
                }

                argna++;
                break;
            }
            case _CALL:{

                int argnc = argna - 4;
                argna = 0;
                saveAllReg();
                struct Code* code1 = newCode(__JAL,-1,-1,-1,p->arg1);
                linkcode(codelist,code1);
                //clean arg%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                int reg3 = convertreg("$sp");
                if (argnc > 0){
                    char offset[20];
                    sprintf(offset,"%d",argnc*4);
                    struct Code* code = newCode(__ADDI,reg3,reg3,-1,offset);
                    linkcode(codelist,code);
                }
                //after clear arg, load $ra,$fp
                int reg4 = convertreg("$ra");
                int reg5 = convertreg("$fp");
                struct Code* code = NULL;
                code = newCode(__LW,reg5,reg3,-1,NULL);
                linkcode(codelist,code);
                code = newCode(__ADDI,reg3,reg3,-1,"4");
                linkcode(codelist,code);
                code = newCode(__LW,reg4,reg3,-1,NULL);
                linkcode(codelist,code);
                code = newCode(__ADDI,reg3,reg3,-1,"4");
                linkcode(codelist,code);

                
                //return value
                int reg1 = getReg(p->target,1);
                int reg2 = convertreg("$v0");
                struct Code* code2 = newCode(__MOVE,reg1,reg2,-1,NULL);
                linkcode(codelist,code2);

                break;
            }
            case _PARAM:{
                /*
                struct Code* code = newCode(__UNDEFINED,-1,-1,-1,NULL);
                linkcode(codelist,code);
                break;
                */
                
                int reg1 = getReg(p->arg1,1);
                if (argna > 4){
                    int reg2 = convertreg("$fp");
                    int reg3 = 24;
                    char offset[20];
                    sprintf(offset,"%d",(argnb-argna)*4);
                    struct Code* code = NULL;
                    code = newCode(__ADDI,reg3,reg2,-1,offset);
                    linkcode(codelist,code);
                    code = newCode(__LW,reg1,reg3,-1,NULL);
                    linkcode(codelist,code);
                }else{
                    char offset[20];
                    sprintf(offset,"$a%d",argna-1);
                    int reg2 = convertreg(offset);
                    struct Code* code = newCode(__MOVE,reg1,reg2,-1,NULL);
                    linkcode(codelist,code);
                }
                argna--;
                
                break;
            }
            case _READ:{
                struct Code* code = NULL;
                int reg1 = convertreg("$sp");
                int reg2 = convertreg("$ra");
                int reg3 = getReg(p->arg1,1);
                int reg4 = convertreg("$v0");
                code = newCode(__ADDI,reg1,reg1,-1,"-4");
                linkcode(codelist,code);
                code = newCode(__SW,reg2,reg1,-1,NULL);
                linkcode(codelist,code);
                code = newCode(__JAL,-1,-1,-1,"read");
                linkcode(codelist,code);
                code = newCode(__LW,reg2,reg1,-1,NULL);
                linkcode(codelist,code);
                code = newCode(__ADDI,reg1,reg1,-1,"4");
                linkcode(codelist,code);
                code = newCode(__MOVE,reg3,reg4,-1,NULL);
                linkcode(codelist,code);
                break;
            }
            case _WRITE:{
                struct Code* code = NULL;
                int reg1 = convertreg("$sp");
                int reg2 = convertreg("$ra");
                int reg3 = convertreg("$a0");
                int reg4 = getReg(p->arg1,0);
                code = newCode(__MOVE,reg3,reg4,-1,NULL);
                linkcode(codelist,code);
                code = newCode(__ADDI,reg1,reg1,-1,"-4");
                linkcode(codelist,code);
                code = newCode(__SW,reg2,reg1,-1,NULL);
                linkcode(codelist,code);
                code = newCode(__JAL,-1,-1,-1,"write");
                linkcode(codelist,code);
                code = newCode(__LW,reg2,reg1,-1,NULL);
                linkcode(codelist,code);
                code = newCode(__ADDI,reg1,reg1,-1,"4");
                linkcode(codelist,code);
                break;
            }
        }
        p = p->next;
    }
    return 0;
}

int init_destination(){
    int i;
    struct Code* code = NULL;
    struct Data* data = NULL;
    for(i=0; i<32; i++) regf[i] = 0;
    codelist = newCode(__TEXT,-1,-1,-1,NULL);
    code = newCode(__FUNC,-1,-1,-1,"read");
    linkcode(codelist,code);
    int reg1 = convertreg("$v0");
    code = newCode(__LI,reg1,-1,-1,"4");
    linkcode(codelist,code);
    int reg2 = convertreg("$a0");
    code = newCode(__LA,reg2,-1,-1,"_prompt");
    linkcode(codelist,code);
    code = newCode(__SYSCALL,-1,-1,-1,NULL);
    linkcode(codelist,code);
    code = newCode(__LI,reg1,-1,-1,"5");
    linkcode(codelist,code);
    code = newCode(__SYSCALL,-1,-1,-1,NULL);
    linkcode(codelist,code);
    int reg3 = convertreg("$ra");
    code = newCode(__JR,reg3,-1,-1,NULL);
    linkcode(codelist,code);
    code = newCode(__FUNC,-1,-1,-1,"write");
    linkcode(codelist,code);
    code = newCode(__LI,reg1,-1,-1,"1");
    linkcode(codelist,code);
    code = newCode(__SYSCALL,-1,-1,-1,NULL);
    linkcode(codelist,code);
    code = newCode(__LI,reg1,-1,-1,"4");
    linkcode(codelist,code);
    code = newCode(__LA,reg2,-1,-1,"_ret");
    linkcode(codelist,code);
    code = newCode(__SYSCALL,-1,-1,-1,NULL);
    linkcode(codelist,code);
    int reg4 = convertreg("$zero");
    code = newCode(__MOVE,reg1,reg4,-1,NULL);
    linkcode(codelist,code);
    code = newCode(__JR,reg3,-1,-1,NULL);
    linkcode(codelist,code);
    
    datalist = newData(__DATA,NULL,NULL);
    data = newData(__ASCIIZ,"_prompt","Enter an integer:");
    linkdata(datalist,data);
    data = newData(__ASCIIZ,"_ret","\\n");
    linkdata(datalist,data);
    data = newData(__GLOBL,"main",NULL);
    linkdata(datalist,data);
    return 0;
}

int print_destination(){
    struct Data* data = datalist;
    while(data != NULL){
        switch(data->type){
            case __DATA: 
                fprintf(dest,".data\n");
                break;
            case __ASCIIZ:
                fprintf(dest,"%s: .asciiz \"%s\"\n",data->name,data->value);
                break;
            case __GLOBL:
                fprintf(dest,".globl %s\n",data->name);
                break;
            default:
                fprintf(dest,"undefined data instr.\n");
        }
        
        data = data->next;
    }
    
    struct Code* code = codelist;
    while(code != NULL){
        switch(code->type){
            case __TEXT:
                fprintf(dest,".text\n");
                break;
            case __LABEL:
                fprintf(dest,"%s:\n",code->arg1);
                break;
            case __LI:
                fprintf(dest,"li $%d, %s\n",code->reg1,code->arg1);
                break;
            case __MOVE:
                fprintf(dest,"move $%d, $%d\n",code->reg1,code->reg2);
                break;
            case __ADDI:
                fprintf(dest,"addi $%d, $%d, %s\n",code->reg1,code->reg2,code->arg1);
                break;
            case __ADD:
                fprintf(dest,"add $%d, $%d, $%d\n",code->reg1,code->reg2,code->reg3);
                break;
            case __SUB:
                fprintf(dest,"sub $%d, $%d, $%d\n",code->reg1,code->reg2,code->reg3);
                break;
            case __MUL:
                fprintf(dest,"mul $%d, $%d, $%d\n",code->reg1,code->reg2,code->reg3);
                break;
            case __DIV:
                fprintf(dest,"div $%d, $%d\n",code->reg1,code->reg2);
                break;
            case __MFLO:
                fprintf(dest,"mflo $%d\n",code->reg1);
                break;
            case __LW:
                fprintf(dest,"lw $%d, 0($%d)\n",code->reg1,code->reg2);
                break;
            case __SW:
                fprintf(dest,"sw $%d, 0($%d)\n",code->reg1,code->reg2);
                break;
            case __J:
                fprintf(dest,"j %s\n",code->arg1);
                break;
            case __JAL:
                fprintf(dest,"jal %s\n",code->arg1);
                break;
            case __JR:
                fprintf(dest,"jr $%d\n",code->reg1);
                break;
            case __BEQ:
                fprintf(dest,"beq $%d, $%d, %s\n",code->reg1,code->reg2,code->arg1);
                break;
            case __BNE:
                fprintf(dest,"bne $%d, $%d, %s\n",code->reg1,code->reg2,code->arg1);
                break;
            case __BGT:
                fprintf(dest,"bgt $%d, $%d, %s\n",code->reg1,code->reg2,code->arg1);
                break;
            case __BLT:
                fprintf(dest,"blt $%d, $%d, %s\n",code->reg1,code->reg2,code->arg1);
                break;
            case __BGE:
                fprintf(dest,"bge $%d, $%d, %s\n",code->reg1,code->reg2,code->arg1);
                break;
            case __BLE:
                fprintf(dest,"ble $%d, $%d, %s\n",code->reg1,code->reg2,code->arg1);
                break;
            case __FUNC:
                fprintf(dest,"%s:\n",code->arg1);
                break;
            case __SYSCALL:
                fprintf(dest,"syscall\n");
                break;
            case __LA:
                fprintf(dest,"la $%d, %s\n",code->reg1,code->arg1);
                break;
            default:
                fprintf(dest,"undefined code instr.type is %d\n",code->type);
        }
        code = code->next;
    }
    
    
    return 0;
}
int destination(){
    init_destination();
    start();
    print_destination();
    fclose(dest);
    return 0;
}
