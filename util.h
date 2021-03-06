#ifndef _UTIL_H_

	#define _UTIL_H_
	#include "instr.h"

	struct Register{
		int name;
		Instruction* firstAppears;
		Instruction* lastAppears;
		int liverange;
		int occurences;
	};

	typedef struct Register Register;

	struct RegSet{
		Register** registers;
		int numRegisters;
	};

	typedef struct RegSet RegSet;

	struct RegClass{
		int size;
		int* name;
		int* next;
		int* free;
		int* stack;
		int* physicalName;
		char* everAllocated;
		int stackTop;
	};

	typedef struct RegClass RegClass;

	// Function declarations
	int classPop(RegClass* class);
	void classPush(RegClass* class, int value);
	void freeRegClass(RegClass* class);
	RegClass* createRegClass(int size);
	void computeLiveRanges(Instruction* head, RegSet* regSet);
	void computeOccurences(Register* reg);
	int findNumRegisters(Instruction* head);
	RegSet* getRegisters(Instruction* head);
	Register* createRegister();
	void destroyRegister(Register* reg);
	RegSet* createRegSet(int numRegisters);
	void destroyRegSet(RegSet* regSet);
	void removeRegFromRegSet(RegSet* regSet, Register* reg);
	void printRegSet(RegSet* set);
	void sortRegSet_liveRanges(RegSet* set);
	void sortRegSet_occurences(RegSet* set);
	void clearRegisterLiveRangesAndRecompute(Instruction* head, RegSet* set);
	void computeRegistersLiveInInstructions(RegSet* set);


#endif