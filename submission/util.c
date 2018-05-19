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

int classPop(RegClass* class){
	int val = class->stack[class->stackTop];
	class->stackTop --;
	return val;
}

void classPush(RegClass* class, int value){
	class->stack[++class->stackTop] = value;
}

void freeRegClass(RegClass* class){
	free(class->name);
	free(class->next);
	free(class->free);
	free(class->stack);
	free(class->physicalName);
	free(class);
}

RegClass* createRegClass(int size){
	RegClass* class = malloc(sizeof(RegClass));
	class->size = size;
	class->name = malloc(sizeof(int) * size);
	class->next = malloc(sizeof(int) * size);
	class->free = malloc(sizeof(int) * size);
	class->stack = malloc(sizeof(int) * size);
	class->physicalName = malloc(sizeof(int) * size);
	class->everAllocated = malloc(sizeof(char) * size);
	class->stackTop = -1;

	int i;
	for(i = 0; i < size; i ++){
		class->name[i] = -1;
		class->next[i] = 0;
		class->free[i] = 1;
		class->physicalName[i] = i + 1;
		class->everAllocated[i] = 0;
		classPush(class, i);
	}

	return class;
}

int rank_liveRanges(Register* a, Register* b){
	if(a->name == 0) return 1;
	if(a->occurences > b->occurences) return 1;
	if(b->occurences > a->occurences) return 0;
	return a->liverange >= b->liverange;
}

// Assumes that liveranges are already set for each register and that unused ones are removed.
// Currently utilizes just an insertion sort.
void sortRegSet_liveRanges(RegSet* set){
	int i, j;
	Register *a, *b;
	for(i = 0; i < set->numRegisters; i ++){
		a = set->registers[i];
		j = i - 1;
		while(j >= 0 && (rank_liveRanges(set->registers[j], a)) ){
			set->registers[j + 1] = set->registers[j];
			j --;
		}
		set->registers[j + 1] = a;
	}
}

// Assumes that occurences are already set for each register and that unused ones are removed.
// Currently utilizes just an insertion sort.
void sortRegSet_occurences(RegSet* set){
	int i, j;
	Register *a, *b;
	for(i = 0; i < set->numRegisters; i ++){
		a = set->registers[i];
		j = i - 1;
		while(j >= 0 && (set->registers[j]->occurences > a->occurences || set->registers[j]->name == 0) ){
			set->registers[j + 1] = set->registers[j];
			j --;
		}
		set->registers[j + 1] = a;
	}
}

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
	
	RegSet* set = createRegSet(numRegs);

	computeLiveRanges(head, set);

	set = reduceRegisterSet(set);

	computeRegistersLiveInInstructions(set);

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

void clearRegisterLiveRangesAndRecompute(Instruction* head, RegSet* set){
	while(head != NULL){
		head->registersLive = 0;
		head = head->next;
	}

	computeRegistersLiveInInstructions(set);
}

void computeRegistersLiveInInstructions(RegSet* set){
	int i = 0;
	Instruction* instr;

	for(i = 0; i < set->numRegisters; i ++){
		if(set->registers[i]->name == 0) continue;
		instr = set->registers[i]->firstAppears;
		while(instr != set->registers[i]->lastAppears){
			instr->registersLive ++;
			instr = instr->next;
		}
	}
}


void computeOccurences(Register* reg){
	if(reg->firstAppears == NULL) return;
	Instruction* start = reg->firstAppears;
	int i = 0;
	while(start != reg->lastAppears){
		for(i = 0; i < start->numArgs; i ++){
			if(start->args[i]->isReg){
				if(start->args[i]->value == reg->name) reg->occurences ++;
			}
		}
		start = start->next;
	}
}

// Instead of shrinking the array, we will simply do a swap with the last element and then set this one to NULL.
void removeRegFromRegSet(RegSet* regSet, Register* reg){
	int i = 0;
	for(i = 0; i < regSet->numRegisters; i ++){
		if(regSet->registers[i] == reg){
			regSet->registers[i] = regSet->registers[regSet->numRegisters - 1];
			regSet->registers[regSet->numRegisters - 1] = NULL;
		}
	}
	regSet->numRegisters --;
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

		if(set->registers[i]->name == 0) continue;

		set->registers[i]->liverange = computeInstructionDepth(def, usage);
		computeOccurences(set->registers[i]);
		if(usage == NULL) continue;
	}

}

Register* createRegister(){
	Register* reg = malloc(sizeof(Register));
	reg->name = 0;
	reg->firstAppears = reg->lastAppears = NULL;
	reg->liverange = reg->occurences = 0;
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
	return;
	int i;

	printf("Register set: %d registers\n", set->numRegisters);
	for(i = 0; i < set->numRegisters; i ++){
		printf("Register %d (liverange: %d, occurences: %d)\n", set->registers[i]->name, set->registers[i]->liverange, set->registers[i]->occurences);

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