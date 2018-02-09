#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "instr.h"

// Function declarations
Arguments* parseArguments(int argc, char** argv);


// Function definitions

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

	Instruction* curr = instr;
	while(curr != NULL){
		printInstruction(stdout, curr);
		curr = curr->next;
	}

	destroyInstructionList(instr);

	return 0;
}