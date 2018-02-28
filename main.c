#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "instr.h"
#include "util.h"

#define TOTAL_REGS (args->numRegs)
#define AVAIL_REGS (TOTAL_REGS - 3)
#define IS_REG_PHYSICAL(O) (((AVAIL_REGS - O) >= 0) ? 1 : 0)
#define GET_OFFSET(O) (getRegisterMemoryStore(O))
#define GET_BOTTOM_UP_OFFSET(O) (getRegisterMemoryStore(O))
#define DEST(I) ((I == 0) ? (AVAIL_REGS + 1) : ( AVAIL_REGS + 2))
#define TOP_DOWN_CLASS_DEST(I) ((I == 0) ? (TOTAL_REGS - 1) : (TOTAL_REGS))
#define INSTR_OCCURS_DEATH 999999999
#define MAX_VIRTUAL_REGISTERS 512

// Function declarations
Arguments* parseArguments(int argc, char** argv);
void process(Arguments* args, Instruction* head, RegSet* registers);
void process_bottomUp(Arguments* args, Instruction* head, RegSet* registers);
void process_topDownClass(Arguments* args, Instruction* head, RegSet* registers);
void process_topDownBook(Arguments* args, Instruction* head, RegSet* registers);
void process_custom(Arguments* args, Instruction* head, RegSet* registers);
int bottomUp_allocate(Arguments* args, Instruction* head, RegClass* class, int vr);
int bottomUp_ensure(Arguments* args, Instruction* head, RegClass* class, int vr);
void bottomUp_free(Arguments* args, Instruction* head, RegClass* class, int i);
Instruction* generateLoadAI(int offset, int destination);
Instruction* generateStoreAI(int offset, int source);
Register* getRegWithValue(RegSet* registers, int value);
int getNextOccurenceDepth(Instruction* a, int regVal);
int getRegisterMemoryStore(int rv);
int getRegisterStore(Arguments* args, int rv);
void unsetRegister(int rv);
int isLastUsage(Instruction* head, int vr);

// Function implementations

int isLastUsage(Instruction* head, int vr){
	int i = 0;
	if(head == NULL) return 1;
	head = head->next;
	while(head != NULL){
		for(i = 0; i < head->numArgs; i ++){
			if(head->args[i]->isReg && head->args[i]->value == vr) return 0;
		}
		head = head->next;
	}
	return 1;
}

// There two variables are used to keep track of where we have been storing variables in the program memory.
int *memoryStoreAddresses = NULL;	// We will end up storing 0 in any rv that has yet to be stored. Later, these will be updated to their real values
int lastMemoryStore = -4;
int getRegisterMemoryStore(int rv){
	int i = 0;
	if(memoryStoreAddresses == NULL){
		memoryStoreAddresses = malloc(sizeof(int) * MAX_VIRTUAL_REGISTERS);
		for(i = 0; i < MAX_REGISTERS; i ++) memoryStoreAddresses[i] = 0;
	}
	
	if(memoryStoreAddresses[rv] == 0){
		memoryStoreAddresses[rv] = lastMemoryStore;
		lastMemoryStore -= 4;
	}

	return memoryStoreAddresses[rv];
}

int *registerStore = NULL;	// We will end up storing 0 in any rv that has yet to be stored. Later, these will be updated to their real values
int *availableRegisterNumbers = NULL;
void unsetRegister(int rv){
	int i = registerStore[rv];
	availableRegisterNumbers[i] = 1;
	registerStore[rv] = -1;
}

int getRegisterStore(Arguments* args, int rv){
	int i = 0;
	if(registerStore == NULL){
		registerStore = malloc(sizeof(int) * MAX_VIRTUAL_REGISTERS);
		for(i = 0; i < MAX_REGISTERS; i ++) registerStore[i] = -1;
		availableRegisterNumbers = malloc(sizeof(int) * MAX_VIRTUAL_REGISTERS);
		for(i = 0; i < MAX_REGISTERS; i ++) availableRegisterNumbers[i] = 1;
		availableRegisterNumbers[0] = 0;
		availableRegisterNumbers[TOP_DOWN_CLASS_DEST(0)] = availableRegisterNumbers[TOP_DOWN_CLASS_DEST(1)] = 0;
	}
	registerStore[0] = 0;
	
	if(registerStore[rv] == -1){
		for(i = 0; i < MAX_VIRTUAL_REGISTERS; i ++){
			if(availableRegisterNumbers[i] == 1){
				registerStore[rv] = i;
				availableRegisterNumbers[i] = 0;
				break;
			}
		}
	}


	return registerStore[rv];
}

int findMaxLive(Instruction* head){
	int i = 0;
	while(head != NULL){
		if(head->registersLive > i) i = head->registersLive;
		head = head->next;
	}
	return i;
}

int findLiveRangeFromSet(RegSet* registers, int vr){
	int i = 0;

	for(i = 0; i < registers->numRegisters; i ++){
		if(registers->registers[i]->name == vr) return registers->registers[i]->liverange;
	}

	return -1;
}

void process_topDownClass(Arguments* args, Instruction* head, RegSet* registers){
	int i = 0, currentRegSetIndex = 0, max = -1, argRange, regVal;
	RegSet *lineRegSet = createRegSet(registers->numRegisters);
	int maxLive = 0;
	InstrArg* arg;
	Instruction* current;
	Register* reg;

	while(1){
		maxLive = findMaxLive(head);
		printf("Maxlive is %d\n", maxLive);
		if(maxLive <= (TOTAL_REGS - 2)) break;

		current = head;
		while(current != NULL){
			max = -1;
			if(current->registersLive <= (TOTAL_REGS - 2)){
				current = current->next;
				continue;
			}

			printInstruction(stdout, current);

			for(i = 0; i < current->numArgs; i ++){
				arg = current->args[i];
				if(!arg->isReg || arg->value == 0) continue;
				argRange = findLiveRangeFromSet(registers, arg->value);
				if(argRange > max) max = argRange;
			}

			for(i = 0; i < current->numArgs; i ++){
				arg = current->args[i];
				if(!arg->isReg || arg->value == 0) continue;
				argRange = findLiveRangeFromSet(registers, arg->value);
				if(argRange == max && max != -1){
					lineRegSet->registers[currentRegSetIndex ++] = getRegWithValue(registers, arg->value);
					removeRegFromRegSet(registers, lineRegSet->registers[currentRegSetIndex - 1]);
					clearRegisterLiveRangesAndRecompute(head, registers);
					break;
				}
			}

			current = current->next;
		}	
	}


	lineRegSet->numRegisters = currentRegSetIndex;
	int destination = 0;


	//printf("REG SET\n");
	//if(DEBUG) printRegSet(registers);
	printf("LINe REG SET\n");
	if(DEBUG) printRegSet(lineRegSet);

	sortRegSet_occurences(registers);

	current = head;
	while(current != NULL){
		destination = 0;

		Instruction* new;
		for(i = 0; i < current->numArgs; i ++){
			arg = current->args[i];
			if(!arg->isReg || arg->value == 0) continue;
			
			// Do a check if it's a store operation or not.
			// If not, we can unset the vr's association with the given register.

			reg = getRegWithValue(lineRegSet, arg->value);
			//printf("%p\n", reg);
			if(reg == NULL){
				reg = getRegWithValue(registers, arg->value);
				regVal = getRegisterStore(args, arg->value);

				if(isLastUsage(current, reg->name)){
					unsetRegister(arg->value);
				}

				arg->value = regVal;
				continue;
			}

			if(current->type == STORE && !arg->isInput){
				new = generateLoadAI(GET_OFFSET(arg->value), TOP_DOWN_CLASS_DEST(0));
				current->last->next = new;
				new->last = current->last;
				current->last = new;
				new->next = current;
				arg->value = TOP_DOWN_CLASS_DEST(0);
				continue;
			}else if(arg->isInput){
				// If we've gotten here, then we need to load the register in from memory.
				new = generateLoadAI(GET_OFFSET(arg->value), TOP_DOWN_CLASS_DEST(destination));
				current->last->next = new;
				new->last = current->last;
				current->last = new;
				new->next = current;
				arg->value = TOP_DOWN_CLASS_DEST(destination);
				destination = 1;
			}else{
				new = generateStoreAI(GET_OFFSET(arg->value), TOP_DOWN_CLASS_DEST(0));
				new->next = current->next;
				new->last = current;
				current->next = new;
				new->next->last = new;

				arg->value = TOP_DOWN_CLASS_DEST(0);

				current = current->next;
				break;
			}
		}

		current = current->next;
	}
}

int getNextOccurenceDepth(Instruction* a, int regVal){
	int depth = 1, i;
	a = a->next;
	while(a != NULL){
		for(i = 0; i < a->numArgs; i ++){
			if(!a->args[i]->isReg) continue;
			if(a->args[i]->value == regVal) return depth;
		}
		depth ++;

		a = a->next;
	}

	return INSTR_OCCURS_DEATH;
}

Register* getRegWithValue(RegSet* registers, int value){
	int i = 0;
	for(i = 0; i < registers->numRegisters; i ++){
		if(registers->registers[i]->name == value) return registers->registers[i];
	}

	return NULL;
}

void bottomUp_free(Arguments* args, Instruction* head, RegClass* class, int i){
	class->name[i] = -1;
	class->next[i] = -1;
	class->free[i] = 1;
	classPush(class, i);
}

int bottomUp_allocate(Arguments* args, Instruction* head, RegClass* class, int vr){
	if(DEBUG) printf("Attempting to allocate register r%d, and we currently have %d registers free\n", vr, class->stackTop + 1);
	int i;
	int j, maxJ = 0;
	Instruction* new;
	if(class->stackTop >= 0) i = classPop(class);
	else{
		for(j = 0; j < class->size; j ++){
			if(class->next[j] > class->next[maxJ] && class->next[j] != -1) maxJ = j;
		}
		if(DEBUG) printf("Found %d [%d, %d] for vr %d, w/ next depth %d to destroy\n", maxJ, class->name[maxJ], class->physicalName[maxJ], vr, class->next[maxJ]);
		i = maxJ;
		new = generateStoreAI(GET_BOTTOM_UP_OFFSET(class->name[i]), class->physicalName[maxJ]);
		head->last->next = new;
		new->last = head->last;
		head->last = new;
		new->next = head;
	}

	class->name[i] = vr;
	class->next[i] = -1;
	class->free[i] = 0;
	class->everAllocated[i] = 1;
	return i;
}

int bottomUp_ensure(Arguments* args, Instruction* head, RegClass* class, int vr){
	int i = 0;
	Instruction* new;
	for(i = 0; i < class->size; i ++){
		if(class->name[i] == vr) return i;
	}

	i = bottomUp_allocate(args, head, class, vr);

	if(1 || class->everAllocated[i]){
		new = generateLoadAI(GET_BOTTOM_UP_OFFSET(vr), class->physicalName[i]);
		head->last->next = new;
		new->last = head->last;
		head->last = new;
		new->next = head;
	}

	class->everAllocated[i] = 1;

	return i;
}

// Function definitions
void process_bottomUp(Arguments* args, Instruction* head, RegSet* registers){
	RegClass* class = createRegClass(args->numRegs);
	int i;
	int rx = -1, ry = -1, rz = -1;
	int numInputs = 0, numOutputs = 0;
	int vri1, vri2, vri3;
	int isOutputValueImportant = 0;

	while(head != NULL){
		isOutputValueImportant = (head->type == STORE || 0);
		numInputs = numOutputs = 0;
		for(i = 0; i < head->numArgs; i ++){
			if(!head->args[i]->isReg) continue;
			if(head->args[i]->isInput){
				if(head->args[i]->value == 0) continue;
				numInputs ++;
				if(numInputs == 1) rx = bottomUp_ensure(args, head, class, head->args[i]->value);
				if(numInputs == 2) ry = bottomUp_ensure(args, head, class, head->args[i]->value);
			}
		}

		vri1 = vri2 = vri3 = -1;

		if(numInputs == 2){
			vri1 = head->args[0]->value;
			vri2 = head->args[1]->value;
		}else if(numInputs == 1){
			if(head->args[0]->isReg && head->args[0]->value != 0) vri1 = head->args[0]->value;
			else vri1 = head->args[1]->value;
		}

		for(i = 0; i < head->numArgs; i ++){
			if(!head->args[i]->isReg) continue;
			if(!head->args[i]->isInput){
				if(head->args[i]->value == 0) continue;
				numOutputs ++;
				if(numOutputs == 1){
					if(!isOutputValueImportant) rz = bottomUp_allocate(args, head, class, head->args[i]->value);
					else rz = bottomUp_ensure(args, head, class, head->args[i]->value);
					vri3 = head->args[i]->value;
				}
			}
		}


		if(vri1 != -1 && getNextOccurenceDepth(head, vri1) == INSTR_OCCURS_DEATH) bottomUp_free(args, head, class, rx);
		if(vri2 != -1 && getNextOccurenceDepth(head, vri2) == INSTR_OCCURS_DEATH) bottomUp_free(args, head, class, ry);

		numInputs = numOutputs = 0;

		for(i = 0; i < head->numArgs; i ++){
			if(!head->args[i]->isReg) continue;
			if(head->args[i]->value == 0) continue;
			if(head->args[i]->isInput){
				numInputs ++;
				if(numInputs == 1){
					head->args[i]->value = class->physicalName[rx];
					class->next[rx] = getNextOccurenceDepth(head, vri1);
				}
				if(numInputs == 2){
					head->args[i]->value = class->physicalName[ry];
					class->next[ry] = getNextOccurenceDepth(head, vri2);
				}
			}else{
				numOutputs ++;
				if(numOutputs == 1){
					head->args[i]->value = class->physicalName[rz];
					class->next[rz] = getNextOccurenceDepth(head, vri3);
				}
			}
		}

		for(i = 0; i < class->size; i ++){
			if(class->next[i] != -1) class->next[i] --;
		}

		head = head->next;

	}

	freeRegClass(class);
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
	int registerReplacements[MAX_REGISTERS];
	for(i = 0; i < MAX_REGISTERS; i ++) registerReplacements[i] = -1;
	// First, we rewrite our register numbers.
	for(i = registers->numRegisters - 1; i >= 0; i --){
		offset = (registers->numRegisters - 1 - i);
		if(offset > AVAIL_REGS) offset += 2;
		registerReplacements[registers->registers[i]->name] = offset;
		if(DEBUG) printf(">> %d => %d\n", registers->registers[i]->name, offset);
	}
	registerReplacements[0] = 0;


	if(DEBUG) printf("Processing: Top Down Processing\n\n\n\n\n\n\n");
	//if(DEBUG) printRegSet(registers);

	while(head != NULL){
		for(i = 0; i < head->numArgs; i ++){
			arg = head->args[i];
			if(!arg->isReg) continue;
			offset = registerReplacements[arg->value];
			// If the offset is within acceptable bounds, then we can go ahead and quit here.
			if(IS_REG_PHYSICAL(offset)){
				arg->value = offset;
				continue;
			}

			if(head->type == STORE) destination = 1;
			if(head->type == STORE && !arg->isInput){
				new = generateLoadAI(GET_OFFSET(offset), DEST(0));
				head->last->next = new;
				new->last = head->last;
				head->last = new;
				new->next = head;
				arg->value = DEST(0);
				continue;
			}

			if(arg->isInput){
				// If we've gotten here, then we need to load the register in from memory.
				new = generateLoadAI(GET_OFFSET(offset), DEST(destination));
				head->last->next = new;
				new->last = head->last;
				head->last = new;
				new->next = head;
				arg->value = DEST(destination);
				destination = 1;
			}else{
				new = generateStoreAI(GET_OFFSET(offset), DEST(0));
				new->next = head->next;
				new->last = head;
				head->next = new;
				new->next->last = new;

				arg->value = DEST(0);

				head = head->next;
				break;
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