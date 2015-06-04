#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

FILE *dest = NULL;
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
struct VarReg{
    char* var;  //v1 or t1
    int reg;    //reg
    struct VarReg* next;
};
struct VarReg* varreg = NULL;
int regf[32];

int getReg(char* varname){
    //first , check varreg list or if a #1, allocate a reg directly
    
    //second , store a reg to memory to get a reg or get a reg immediately
    
    //third , allocate this reg to varname
    
    return 0;
}
int convertreg(char* name){
    if (strcmp(name,"$zero") == 0) return 0;
    if (strcmp(name,"$at") == 0) return 1;
    if (strcmp(name,"$v0") == 0) return 2;
    if (strcmp(name,"$gp") == 0) return 28;
    if (strcmp(name,"$sp") == 0) return 29;
    if (strcmp(name,"$fp") == 0) return 30;
    if (strcmp(name,"$ra") == 0) return 31;
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
                struct Code* code = newCode(__LABEL,-1,-1,-1,p->arg1);
                linkcode(codelist,code);
                break;
            }
            case _FUNCTION:{
                //fprintf(file,"FUNCTION %s :\n",instrlist->arg1);
                break;
            }
            case _ASSIGNOP:{
                if(p->arg1[0] != '#'){
                    int reg1 = getReg(p->target);
                    int reg2 = getReg(p->arg1);
                    struct Code* code = newCode(__MOVE,reg1,reg2,-1,NULL);
                    linkcode(codelist,code);
                }else{
                    int reg1 = getReg(p->target);
                    struct Code* code = newCode(__LI,reg1,-1,-1,p->arg1);
                    linkcode(codelist,code);
                }
                break;
            }
            case _ADD:{
                int reg0 = getReg(p->target);
                int reg1 = getReg(p->arg1);
                if (p->arg2[0] != '#'){
                    int reg2 = getReg(p->arg2);
                    struct Code* code = newCode(__ADD,reg0,reg1,reg2,NULL);
                    linkcode(codelist,code);
                }else{
                    struct Code* code = newCode(__ADDI,reg0,reg1,-1,p->arg2);
                    linkcode(codelist,code);
                }
                break;
            }
            case _MINUS:{
                int reg0 = getReg(p->target);
                int reg1 = getReg(p->arg1);
                if (p->arg2[0] != '#'){
                    int reg2 = getReg(p->arg2);
                    struct Code* code = newCode(__SUB,reg0,reg1,reg2,NULL);
                    linkcode(codelist,code);
                }else{
                    char arg[20];
                    sprintf(arg,"-%s",p->arg2);
                    struct Code* code = newCode(__ADDI,reg0,reg1,-1,arg);
                    linkcode(codelist,code);
                }
                break;
            }
            case _STAR:{
                int reg0 = getReg(p->target);
                int reg1 = getReg(p->arg1);
                int reg2 = getReg(p->arg2);
                struct Code* code = newCode(__MUL,reg0,reg1,reg2,NULL);
                linkcode(codelist,code);
                break;
            }
            case _DIV:{
                int reg0 = getReg(p->target);
                int reg1 = getReg(p->arg1);
                int reg2 = getReg(p->arg2);
                struct Code* code1 = newCode(__DIV,reg1,reg2,-1,NULL);
                struct Code* code2 = newCode(__MFLO,reg0,-1,-1,NULL);
                linkcode(codelist,linkcode(code1,code2));
                break;
            }
            case _GETADDR:{
                //fprintf(file,"%s := &%s\n",instrlist->target,instrlist->arg1);
                break;
            }
            case _GETVALUE:{
                int reg1 = getReg(p->target);
                int reg2 = getReg(p->arg1);
                struct Code* code = newCode(__LW,reg1,reg2,-1,NULL);
                linkcode(codelist,code);
                break;
            }
            case _ASSIGNOP_GETVALUE:{
                int reg1 = getReg(p->arg1);
                int reg2 = getReg(p->target);
                struct Code* code = newCode(__SW,reg1,reg2,-1,NULL);
                linkcode(codelist,code);
                break;
            }
            case _GOTO:{
                struct Code* code = newCode(__J,-1,-1,-1,p->arg1);
                linkcode(codelist,code);
                break;
            }
            case _IF:{
                struct Code* code = NULL;
                int reg1 = getReg(p->arg1);
                int reg2 = getReg(p->arg2);
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
                break;
            }
            case _RETURN:{
                int reg1 = getReg(p->arg1);
                int reg2 = convertreg("$vo");
                int reg3 = convertreg("$ra");
                struct Code* code1 = newCode(__MOVE,reg2,reg1,-1,NULL);
                struct Code* code2 = newCode(__JR,reg3,-1,-1,NULL);
                linkcode(codelist,linkcode(code1,code2));
                break;
            }
            case _DEC:{
                //fprintf(file,"DEC %s %s\n",instrlist->arg1,instrlist->arg2);
                break;
            }
            case _ARG:{
                //fprintf(file,"ARG %s\n",instrlist->arg1);
                break;
            }
            case _CALL:{
                struct Code* code1 = newCode(__JAL,-1,-1,-1,p->arg1);
                int reg1 = getReg(p->target);
                int reg2 = convertreg("$v0");
                struct Code* code2 = newCode(__MOVE,reg1,reg2,-1,NULL);
                linkcode(codelist,linkcode(code1,code2));
                break;
            }
            case _PARAM:{
                //fprintf(file,"PARAM %s\n",instrlist->arg1);
                break;
            }
            case _READ:{
                //fprintf(file,"READ %s\n",instrlist->arg1);
                break;
            }
            case _WRITE:{
                //fprintf(file,"WRITE %s\n",instrlist->arg1);
                break;
            }
        }
        p = p->next;
    }
    return 0;
}

int init_destination(){
    int i;
    dest = fopen("dest.s","w+");
    struct Code* code = NULL;
    struct Data* data = NULL;
    for(i=0; i<32; i++) regf[i] = 0;
    codelist = newCode(__TEXT,-1,-1,-1,NULL);
    code = newCode(__FUNC,-1,-1,-1,"read");
    linkcode(codelist,code);
    int reg1 = convertreg("$v0");
    code = newCode(__LI,reg1,-1,-1,"#4");
    linkcode(codelist,code);
    int reg2 = convertreg("$a0");
    code = newCode(__LA,reg2,-1,-1,"_prompt");
    linkcode(codelist,code);
    code = newCode(__SYSCALL,-1,-1,-1,NULL);
    linkcode(codelist,code);
    code = newCode(__LI,reg1,-1,-1,"#5");
    linkcode(codelist,code);
    code = newCode(__SYSCALL,-1,-1,-1,NULL);
    linkcode(codelist,code);
    int reg3 = convertreg("$ra");
    code = newCode(__JR,reg3,-1,-1,NULL);
    linkcode(codelist,code);
    code = newCode(__FUNC,-1,-1,-1,"write");
    linkcode(codelist,code);
    code = newCode(__LI,reg1,-1,-1,"#1");
    linkcode(codelist,code);
    code = newCode(__SYSCALL,-1,-1,-1,NULL);
    linkcode(codelist,code);
    code = newCode(__LI,reg1,-1,-1,"#4");
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
    data = newData(__ASCIIZ,"_ret","\n");
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
                fprintf(dest,"li %d, %s\n",code->reg1,code->arg1);
                break;
            case __MOVE:
                fprintf(dest,"move %d, %d\n",code->reg1,code->reg2);
                break;
            case __ADDI:
                fprintf(dest,"addi %d, %d, %s\n",code->reg1,code->reg2,code->arg1);
                break;
            case __ADD:
                fprintf(dest,"add %d, %d, %d\n",code->reg1,code->reg2,code->reg3);
                break;
            case __SUB:
                fprintf(dest,"sub %d, %d, %d\n",code->reg1,code->reg2,code->reg3);
                break;
            case __MUL:
                fprintf(dest,"mul %d, %d, %d\n",code->reg1,code->reg2,code->reg3);
                break;
            case __DIV:
                fprintf(dest,"div %d, %d\n",code->reg1,code->reg2);
                break;
            case __MFLO:
                fprintf(dest,"mflo %d\n",code->reg1);
                break;
            case __LW:
                
            default:
                fprintf(dest,"undefined code instr.\n");
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
