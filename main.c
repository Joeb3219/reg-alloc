#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "instr.h"
#include "util.h"

// Function declarations
Arguments* parseArguments(int argc, char** argv);
Instruction* process(Arguments* args, Instruction* head, RegSet* registers);
Instruction* process_bottomUp(Arguments* args, Instruction* head, RegSet* registers);
Instruction* process_topDownClass(Arguments* args, Instruction* head, RegSet* registers);
Instruction* process_topDownBook(Arguments* args, Instruction* head, RegSet* registers);
Instruction* process_custom(Arguments* args, Instruction* head, RegSet* registers);

// Function definitions
Instruction* process_bottomUp(Arguments* args, Instruction* head, RegSet* registers){
	return head;
}

Instruction* process_topDownClass(Arguments* args, Instruction* head, RegSet* registers){
	return head;
}

Instruction* process_topDownBook(Arguments* args, Instruction* head, RegSet* registers){
	if(DEBUG) printf("Processing: Top Down Processing\n\n\n\n\n\n\n");

	return head;
}

Instruction* process_custom(Arguments* args, Instruction* head, RegSet* registers){
	return head;
}


Instruction* process(Arguments* args, Instruction* head, RegSet* registers){
	if(args->allocationType == TOP_DOWN_BOOK) return process_topDownBook(args, head, registers);
	else if(args->allocationType == TOP_DOWN_CLASS) return process_topDownClass(args, head, registers);
	else if(args->allocationType == BOTTOM_UP) return process_bottomUp(args, head, registers);
	else if(args->allocationType == CUSTOM) return process_custom(args, head, registers);
	
	printf("Invalid allocator type\n"); 
	return NULL;
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

	Instruction* result = process(args, instr->next, registers);
	Instruction* curr = result;

	// Output
	FILE* output = fopen("samples/out.i", "w");
	while(curr != NULL){
		printInstruction(output, curr);
		curr = curr->next;
	}
	fclose(output);

	if(result != instr) destroyInstructionList(result);
	destroyInstructionList(instr);

	return 0;
}