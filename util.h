#ifndef _UTIL_H_

	#define _UTIL_H_
	#include "instr.h"

	struct Register{
		int name;
		Instruction* firstAppears;
		Instruction* lastAppears;
		int liverange;
	};

	typedef struct Register Register;

	struct RegSet{
		Register** registers;
		int numRegisters;
	};

	typedef struct RegSet RegSet;

	// Function declarations
	void computeLiveRanges(Instruction* head, RegSet* regSet);
	int findNumRegisters(Instruction* head);
	RegSet* getRegisters(Instruction* head);
	Register* createRegister();
	void destroyRegister(Register* reg);
	RegSet* createRegSet(int numRegisters);
	void destroyRegSet(RegSet* regSet);
	void printRegSet(RegSet* set);
	void sortRegSet_liveRanges(RegSet* set);


#endif