#ifndef INSTR_H_

	#define INSTR_H_
	#define INSTR_INPUT 	1
	#define INSTR_OUTPUT 	0
	#define INSTR_REG 		1
	#define INSTR_VALUE 	0

	enum InstrType{
		OUTPUTI, OUTPUTAI, OUTPUT,
		LOADI, LOADAI, LOAD,
		STOREI, STOREAI, STORE,
		ADD, MULT, DIV, SUB,
		ADDI, MULTI, DIVI, SUBI,
		LSHIFT, RSHIFT,
		ERROR
	};

	typedef enum InstrType InstrType;

	struct InstrArg{
		int isInput;	// Either INSTR_INPUT or INSTR_OUTPUT.
		int isReg;		// Either INSTR_REG or INSTR_VALUE.
		int value;		// The value of the argument, which is either the register number, or the immediate value.
	};

	typedef struct InstrArg InstrArg;

	struct Instruction{
		InstrType type;
		int numArgs;
		InstrArg** args;
		int registersLive;
		struct Instruction* next;
		struct Instruction* last;
	};

	typedef struct Instruction Instruction;

	// Function declarations
	InstrArg* createInstrArg();
	void destroyInstrArg(InstrArg* arg);
	char* typeToStr(InstrType type);
	InstrType strToType(char* str);
	void decodeInstruction(char* line, Instruction* instr);
	void printInstruction(FILE* file, Instruction* instr);
	Instruction* createInstruction();
	void destroyInstruction(Instruction* instr);
	void destroyInstructionList(Instruction* instr);
	Instruction* getInstructions(char* fileName);
	

#endif