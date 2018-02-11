#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "instr.h"
#include "main.h"

// Location declarations
Instruction* findDefinition(Instruction* head, int reg);
Instruction* findUsage(Instruction* end, int reg);
int computeInstructionDepth(Instruction* a, Instruction* b);
RegSet* reduceRegisterSet(RegSet* original);

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

RegSet* getRegisters(Instruction* head){
	int numRegs = findNumRegisters(head), i;
	if(DEBUG) printf("Found %d regs\n", numRegs);
	
	RegSet* set = createRegSet(numRegs);

	computeLiveRanges(head, set);

	set = reduceRegisterSet(set);

	printRegSet(set);

	return set;
}

// Will scan for the definition of a register.
Instruction* findDefinition(Instruction* head, int reg){
	int i = 0;
	while(head != NULL){
		for(i = 0; i < head->numArgs; i ++){
			if(head->args[i]->isReg && !head->args[i]->isInput){
				if(head->args[i]->value == reg) return head;
			}
		}
		head = head->next;
	}
	return NULL;
}

// Will scan for the last usage of a register.
Instruction* findUsage(Instruction* end, int reg){
	int i = 0;
	while(end != NULL){
		for(i = 0; i < end->numArgs; i ++){
			if(end->args[i]->isReg && end->args[i]->isInput){
				if(end->args[i]->value == reg) return end;
			}
		}
		end = end->last;
	}
	return NULL;
}

int computeInstructionDepth(Instruction* a, Instruction* b){
	int depth = 0;
	while(a != NULL && a != b){
		depth ++;
		a = a->next;
	}
	return depth;
}


void computeLiveRanges(Instruction* head, RegSet* set){
	int i = 0, depth;

	Instruction* def, *usage;
	Instruction* end = head;
	while(end->next != NULL){
		end = end->next;
	}

	for(i = 0; i < set->numRegisters; i ++){
		set->registers[i]->firstAppears = def = findDefinition(head, set->registers[i]->name);
		set->registers[i]->lastAppears = usage = findUsage(end, set->registers[i]->name);

		if(def == NULL){
			if(DEBUG) printf("REGISTER %d NEVER DEFINED!\n", set->registers[i]->name);
		}

		set->registers[i]->liverange = computeInstructionDepth(def, usage);

		if(usage == NULL) continue;
	}

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

// This function produces a register set that removes any unused registers.
RegSet* reduceRegisterSet(RegSet* original){
	int numUsed = 0, i;

	for(i = 0; i < original->numRegisters; i ++){
		if(original->registers[i]->firstAppears == NULL && original->registers[i]->lastAppears == NULL && i != 0) continue;
		numUsed ++;
	}

//	if(numUsed == original->numRegisters) return original;

	RegSet* newSet = createRegSet(numUsed);

	numUsed = 0;
	for(i = 0; i < original->numRegisters; i ++){
		if(original->registers[i]->firstAppears == NULL && original->registers[i]->lastAppears == NULL && i != 0) continue;
		newSet->registers[numUsed ++] = original->registers[i];
	}

	free(original->registers);
	free(original);
	return newSet;
}

RegSet* createRegSet(int numRegisters){
	int i;
	RegSet* set = malloc(sizeof(RegSet));

	set->numRegisters = numRegisters;
	set->registers = malloc(sizeof(Register*) * numRegisters);
	for(i = 0; i < numRegisters; i ++){
		set->registers[i] = createRegister();
		set->registers[i]->name = i;
	}

	return set;
}

void destroyRegSet(RegSet* regSet){
	int i;
	for(i = 0; i < regSet->numRegisters; i ++){
		destroyRegister(regSet->registers[i]);
	}

	free(regSet->registers);
	free(regSet);
}

void printRegSet(RegSet* set){
	int i;

	printf("Register set: %d registers\n", set->numRegisters);
	for(i = 0; i < set->numRegisters; i ++){
		printf("Register %d (liverange: %d)\n", set->registers[i]->name, set->registers[i]->liverange);

		if(set->registers[i]->firstAppears == NULL) printf("\tReg never defined\n");
		else{
			printf("\tFirst defined: ");	
			printInstruction(stdout, set->registers[i]->firstAppears);
		}
		if(set->registers[i]->lastAppears == NULL) printf("\tReg never used\n");
		else{
			printf("\tLast used: ");	
			printInstruction(stdout, set->registers[i]->lastAppears);
		}
	}

}