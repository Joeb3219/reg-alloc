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

	// Function declarations
	int findNumRegisters(Instruction* head);
	Register** getRegisters(Instruction* head);
	Register* createRegister();
	void destroyRegister(Register* reg);

#endif