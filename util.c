#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "instr.h"
#include "main.h"


int findNumRegisters(Instruction* head){
	int i = 0, argI = 0;
	while(head != NULL){
		for(argI = 0; argI < head->numArgs; argI ++){
			if(head->args[argI]->isReg){
				if(head->args[argI]->value > i) i = head->args[argI]->value;
			}
		}
		head = head->next;
	}

	return i + 1;
}

Register** getRegisters(Instruction* head){
	int numRegs = findNumRegisters(head);
	printf("Found %d regs\n", numRegs);
	return NULL;
}

Register* createRegister(){
	Register* reg = malloc(sizeof(Register));
	reg->name = 0;
	reg->firstAppears = reg->lastAppears = NULL;
	reg->liverange = 0;
	return reg;
}

void destroyRegister(Register* reg){
	free(reg);
}
