#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "instr.h"
#include "main.h"

Instruction* createInstruction(){
	Instruction* instr = malloc(sizeof(Instruction));
	instr->type = ERROR;
	instr->numArgs = 0;
	instr->args = malloc(sizeof(InstrArg*) * 4);
	instr->next = instr->last = NULL;
	instr->registersLive = 0;
	return instr;
}

void destroyInstruction(Instruction* instr){
	free(instr);
}

void destroyInstructionList(Instruction* instr){
	if(instr->next == NULL) destroyInstruction(instr);
	else{
		destroyInstructionList(instr->next);
		destroyInstruction(instr);
	}
}

Instruction* processLine(Instruction* base, char* line){
	Instruction* newInstr = createInstruction();
	decodeInstruction(line, newInstr);

	if(newInstr->type == ERROR){
		destroyInstruction(newInstr);
		return base;
	}

	base->next = newInstr;
	newInstr->last = base;
	return newInstr;
}

void findNextToken(char** str, char* buffer){
	// Get the string at the given address.
	char* rStr = *str;
	int foundLeftEdge = 0;
	int i = 0;
	while(rStr[0] != '\0'){
		// Scan through characters until we find one that isn't whitespace.
		if(!foundLeftEdge){
			if(rStr[0] == '\t' || rStr[0] == ' '){
				rStr ++;
				continue;
			}else{
				buffer[i++] = rStr[0];
				foundLeftEdge = 1;
			}
		// Now scan throug until we hit the next whitespace.
		// If we hit \0, then we'll technically be at an edge, too, and this will stop through the loop stopping.
		}else{
			if(rStr[0] == '\t' || rStr[0] == ' ') break;
			buffer[i++] = rStr[0];
		}
		// Move to next character
		rStr ++;
	}
	// Cap off our string.
	buffer[i++] = '\0';
	str[0] = rStr;
}

void decodeInstruction(char* line, Instruction* instr){
	int i = 0, j = 0;
	char* instructionType;
	// At most, we can have an argument that takes up the whole line.
	char buffer[strlen(line) + 1];
	InstrArg* arg;

	findNextToken(&line, buffer);
	instr->type = strToType(buffer);

	int processingInputs = 1;
	while(line[0] != '\0'){
		findNextToken(&line, buffer);

		// Determine if we are now on the right side of the => sign, which means that all other arguments are outputs.
		if(strcmp(buffer, "=>") == 0){
			processingInputs = 0;
			continue;
		}

		// Build up the argument for what it is.

		arg = createInstrArg();
		arg->isInput = processingInputs;
		arg->isReg = buffer[0] == 'r';

		if(arg->isReg) arg->value = atoi(&buffer[1]);
		else arg->value = atoi(buffer);
	
		instr->args[instr->numArgs ++] = arg;
	}

}

InstrArg* createInstrArg(){
	InstrArg* arg = malloc(sizeof(InstrArg));
	arg->isInput = arg->isReg = arg->value = 0;
	return arg;
}

void destroyInstrArg(InstrArg* arg){
	free(arg);
}

void printInstruction(FILE* file, Instruction* instr){
	int i;

	fprintf(file, "%s", typeToStr(instr->type));

	InstrArg* arg;
	int printedYields = 0;
	for(i = 0; i < instr->numArgs; i ++){
		arg = instr->args[i];
		if(!arg->isInput && !printedYields){
			fprintf(file, "\t=>");
			printedYields = 1;
		}

		if(arg->isReg) fprintf(file, "\tr%d", arg->value);
		else fprintf(file, "\t%d", arg->value);
		if(i != instr->numArgs - 1){
			if(instr->args[i]->isInput == instr->args[i + 1]->isInput) fprintf(file, ",");
		}
	}
	
	fprintf(file, "\n");
}

char* typeToStr(InstrType type){
	switch(type){
		case OUTPUT: return "output";
		case OUTPUTI: return "outputI";
		case OUTPUTAI: return "outputAI";
		case STOREAI: return "storeAI";
		case STORE: return "store";
		case STOREI: return "storeI";
		case LOADAI: return "loadAI";
		case LOADI: return "loadI";
		case LOAD: return "load";
		case MULT: return "mult";
		case DIV: return "div";
		case ADD: return "add";
		case SUB: return "sub";
		case MULTI: return "multI";
		case DIVI: return "divI";
		case ADDI: return "addI";
		case SUBI: return "subI";
		default: return "ERROR";
	}
}

InstrType strToType(char* str){
	if(str == NULL) return ERROR;
	if(strcmp("output", str) == 0) return OUTPUT;
	if(strcmp("outputI", str) == 0) return OUTPUTI;
	if(strcmp("outputAI", str) == 0) return OUTPUTAI;
	if(strcmp("storeI", str) == 0) return STOREI;
	if(strcmp("store", str) == 0) return STORE;
	if(strcmp("storeAI", str) == 0) return STOREAI;
	if(strcmp("loadI", str) == 0) return LOADI;
	if(strcmp("load", str) == 0) return LOAD;
	if(strcmp("loadAI", str) == 0) return LOADAI;
	if(strcmp("mult", str) == 0) return MULT;
	if(strcmp("div", str) == 0) return DIV;
	if(strcmp("add", str) == 0) return ADD;
	if(strcmp("sub", str) == 0) return SUB;
	if(strcmp("divI", str) == 0) return DIVI;
	if(strcmp("multI", str) == 0) return MULTI;
	if(strcmp("addI", str) == 0) return ADDI;
	if(strcmp("subI", str) == 0) return SUBI;
	return ERROR;
}

Instruction* getInstructions(char* fileName){
	FILE* file = fopen(fileName, "r");
	if(file == NULL){
		fprintf(stderr, "No such file: %s\n", fileName);
		exit(1);
	}

	char c;
	char buff[256];
	int i = 0;
	Instruction* instruction = createInstruction();
	Instruction* head = instruction;
	while((c = fgetc(file)) != EOF){
		if(c == '\r' || c == '\n'){
			if(i == 0) continue;
			buff[i++] = '\0';
			instruction = processLine(instruction, buff);
			i = 0;
		}else{
			buff[i++] = c;
		}
	}

	fclose(file);
	return head;
}
