#ifndef _MAIN_H_

	#define _MAIN_H_
	#define DEBUG 1

	enum AllocatorType{
		TOP_DOWN_CLASS, TOP_DOWN_BOOK, BOTTOM_UP, CUSTOM
	};

	typedef enum AllocatorType AllocatorType;

	struct Arguments{
		int numRegs;
		AllocatorType allocationType;
		char* inputFileName;
	};

	typedef struct Arguments Arguments;

#endif