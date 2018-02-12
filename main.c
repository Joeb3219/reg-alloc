#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "instr.h"
#include "util.h"

#define TOTAL_REGS (args->numRegs)
#define AVAIL_REGS (TOTAL_REGS - 3)
#define IS_REG_PHYSICAL(O) (((AVAIL_REGS - O) >= 0) ? 1 : 0)
#define GET_OFFSET(O) ((O - AVAIL_REGS + 1) * 4)
#define DEST(I) ((I == 0) ? (I + AVAIL_REGS) : ( I + AVAIL_REGS + 1))

// Function declarations
Arguments* parseArguments(int argc, char** argv);
void process(Arguments* args, Instruction* head, RegSet* registers);
void process_bottomUp(Arguments* args, Instruction* head, RegSet* registers);
void process_topDownClass(Arguments* args, Instruction* head, RegSet* registers);
void process_topDownBook(Arguments* args, Instruction* head, RegSet* registers);
void process_custom(Arguments* args, Instruction* head, RegSet* registers);
int getRegisterPositionFromEnd(RegSet* set, int name);

// Function definitions
int getRegisterPositionFromEnd(RegSet* set, int name){
	int i = 0, j = 0;
	for(i = set->numRegisters - 1; i >= 0; i --){
		if(set->registers[i]->name == name) return j;
		j ++;
	}

	return -1;
}

void process_bottomUp(Arguments* args, Instruction* head, RegSet* registers){
	
}

void process_topDownClass(Arguments* args, Instruction* head, RegSet* registers){
	
}

Instruction* generateLoadAI(int offset, int destination){
	Instruction* instr = createInstruction();

	instr->type = LOADAI;
	instr->numArgs = 3;
	instr->args[0] = createInstrArg();
	instr->args[1] = createInstrArg();
	instr->args[2] = createInstrArg();

	instr->args[0]->isInput = instr->args[1]->isInput = 1;
	instr->args[2]->isInput = 0;

	instr->args[0]->isReg = instr->args[2]->isReg = 1;
	instr->args[1]->isReg = 0;

	instr->args[0]->value = 0;
	instr->args[1]->value = offset;
	instr->args[2]->value = destination;

	return instr;
}

Instruction* generateStoreAI(int offset, int source){
	Instruction* instr = createInstruction();

	instr->type = STOREAI;
	instr->numArgs = 3;
	instr->args[0] = createInstrArg();
	instr->args[1] = createInstrArg();
	instr->args[2] = createInstrArg();

	instr->args[0]->isInput = 1;
	instr->args[2]->isInput = instr->args[1]->isInput = 0;

	instr->args[0]->isReg = instr->args[1]->isReg = 1;
	instr->args[2]->isReg = 0;

	instr->args[0]->value = source;
	instr->args[1]->value = 0;
	instr->args[2]->value = offset;

	return instr;
}

void process_topDownBook(Arguments* args, Instruction* head, RegSet* registers){
	int i, offset, destination = 0;
	InstrArg *arg;
	Instruction *new;
	sortRegSet_occurences(registers);

	if(DEBUG) printf("Processing: Top Down Processing\n\n\n\n\n\n\n");
	if(DEBUG) printRegSet(registers);

	while(head != NULL){
		for(i = 0; i < head->numArgs; i ++){
			arg = head->args[i];
			if(!arg->isReg) continue;
			offset = getRegisterPositionFromEnd(registers, arg->value);
			// If the offset is within acceptable bounds, then we can go ahead and quit here.
			if(IS_REG_PHYSICAL(offset)) continue;
			if(arg->isInput){
				// If we've gotten here, then we need to load the register in from memory.
				new = generateLoadAI(GET_OFFSET(offset), DEST(destination));
				head->last->next = new;
				new->last = head->last;
				head->last = new;
				new->next = head;
				destination = 1;

				arg->value = DEST(destination);
			}else{
				new = generateStoreAI(GET_OFFSET(offset), DEST(0));
				head->last->next = new;
				new->last = head->last;
				head->last = new;
				new->next = head;

				arg->value = DEST(0);
			}		
		}

		destination = 0;
		head = head->next;
	}	
}

void process_custom(Arguments* args, Instruction* head, RegSet* registers){
	
}


void process(Arguments* args, Instruction* head, RegSet* registers){
	if(registers->numRegisters <= AVAIL_REGS) printf("Program used %d registers, which fit within %d physical registers.\n", registers->numRegisters, AVAIL_REGS);
	else if(args->allocationType == TOP_DOWN_BOOK) process_topDownBook(args, head, registers);
	else if(args->allocationType == TOP_DOWN_CLASS) process_topDownClass(args, head, registers);
	else if(args->allocationType == BOTTOM_UP) process_bottomUp(args, head, registers);
	else if(args->allocationType == CUSTOM) process_custom(args, head, registers);
	else printf("Invalid allocator type\n"); 
}

// Will create a struct full of all of the data passed in from the command line.
Arguments* parseArguments(int argc, char** argv){
	if(argc != 4) return NULL;
	Arguments* args = malloc(sizeof(Arguments));
	args->numRegs = atoi(argv[1]);
	args->inputFileName = argv[3];

	// If argv[2] is b, we use bottom up, s for book T-D, t for class T-D, o for custom.
	switch(argv[2][0]){
		case 'b':
			args->allocationType = BOTTOM_UP;
			break;
		case 's':
			args->allocationType = TOP_DOWN_BOOK;
			break;
		case 't':
			args->allocationType = TOP_DOWN_CLASS;
			break;
		default:
			args->allocationType = CUSTOM;
			break;
	}
	return args;
}

int main(int argc, char** argv){
	Arguments* args = parseArguments(argc, argv);
	if(args == NULL){
		fprintf(stderr, "Invalid argument count\n");
		return 1;
	}

	if(DEBUG){
		printf("============================================\n");
		printf("====\t\tArgument List\t\t====\n");
		printf("Number of registers:\t%d\n", args->numRegs);
		printf("Type of allocator:\t%d\n", args->allocationType);
		printf("Input File Name:\t%s\n", args->inputFileName);
		printf("====\t\tArgument List\t\t====\n");
		printf("============================================\n");
	}

	Instruction* instr = getInstructions(args->inputFileName);

	RegSet* registers = getRegisters(instr);

	process(args, instr, registers);
	Instruction* curr = instr->next;

	// Output
	FILE* output = fopen("samples/out.i", "w");
	while(curr != NULL){
		printInstruction(output, curr);
		curr = curr->next;
	}
	fclose(output);

	destroyInstructionList(instr);

	return 0;
}