#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_TEST_FILES 12
#define NUM_TEST_REGS 3
#define NUM_TESTING_SYMBOLS 3

char *files[NUM_TEST_FILES] = {"blocks/block1.i",	"blocks/block2.i",	"blocks/block3.i",	"blocks/block4.i",	"blocks/block5.i",	"blocks/block6.i",	"blocks/report1.i",							"blocks/report2.i",	"blocks/report3.i",	"blocks/report5.i",	"blocks/report4.i",		"blocks/report6.i"};
char *fileArgs[NUM_TEST_FILES] = {"-i 1024 1 1",	"",					"-i 1024 1 1",	"-i 1024 1 1",	"-i 1024 0 1",	"-i 1024 0 1",	"-i 4000 0 10 20 30 40 50 60 70 80 90", "-i 1024 0 1",	"-i 1024 0 1",	"-i 1024 1",	"-i 1024 0 1 2",	"-i 1024 0 1"};
int numTestRegisters[NUM_TEST_REGS] = {5, 10, 20};
char testingSymbols[NUM_TESTING_SYMBOLS] = {'b', 't', 's'};

struct Results{
	char* fileName;
	char allocator;
	int numRegs;
	int numCycles;
	char** results;
	int numResultLines;
	int resultsCorrect;
	float executionTime;
};

typedef struct Results Results;

int areResultsEqual(Results* a, Results* b){
	if(a->numResultLines != b->numResultLines) return 0;

	int i;
	for(i = 0; i < a->numResultLines; i ++){
		if(strcmp(a->results[i], b->results[i]) != 0) return 0;
	}
	return 1;
}

char* getFileArgs(char* fileName){
	int i;
	for(i = 0; i < NUM_TEST_FILES; i ++){
		if(strcmp(files[i], fileName) == 0) return fileArgs[i];
	}
	return "";
}

void fetchResults(Results* results){
	char c;
	char* line;
	int i = 0;

	if(results->allocator == -1){
		if(fork() == 0){
			execl("testSim.sh", "testSim.sh", results->fileName, "results", getFileArgs(results->fileName), NULL);	
		}else{
			wait(NULL);
		}
	}else{
		//printf("Getting results of %s %s\n", results->fileName, getFileArgs(results->fileName));
		if(fork() == 0){
			char buff[8];
			sprintf(buff, "%d\0", results->numRegs);
			char buff2[2];
			buff2[0] = results->allocator;
			buff2[1] = '\0';
			execl("/home/joeb3219/code/cs415/register/test/timeAlloc.sh", "timeAlloc.sh", buff, buff2, results->fileName, NULL);			
		}else{
			wait(NULL);
		}

		if(fork() == 0){
			execl("testSim.sh", "testSim.sh", "../samples/out.i", "results", getFileArgs(results->fileName), NULL);			
		}else{
			wait(NULL);
		}

		// Now fetch how long it took to build the allocation.
		FILE* allTime = fopen("results_time", "r");
		line = malloc(sizeof(char) * 128);
		i = -1;

		while((c = fgetc(allTime)) != EOF){
			if(c == 'r') i = 0;
			if(c == '\n' || c == '\r'){
				line[i++] = '\0';
				break;
			}
			if(i >= 0){
				line[i++] = c;
			}
		}
		int executionTimeMinutes = 0;
		float executionTimeSeconds = 0;
		sscanf(line, "real %dm%fs", &executionTimeMinutes, &executionTimeSeconds);
		results->executionTime = (executionTimeMinutes* 60) + executionTimeSeconds;
	}

	FILE* read = fopen("results", "r");
	line = malloc(sizeof(char) * 256);
	int j;
	int size = 0;
	i = 0;

	while( (c = fgetc(read)) != EOF){
		if(c == '\n' || c == '\r'){
			if(i == 0) continue;
			line[i++] = '\0';
			if(line[0] == 'E'){
				j = i;
				while(line[--i] != ' ');
				while(line[--i] != ' ');
				j = i;
				while(line[++j] != ' ');
				size = j - i;
				char cyclesBuff[16];
				for(i ++; i < j; i ++){
					cyclesBuff[i - j + size - 1] = line[i];
				}
				cyclesBuff[size] = '\0';
				results->numCycles = atoi(cyclesBuff);
			}else if((line[0] >= '0' && line[0] <= '9') || line[0] == -1){
				results->results[results->numResultLines++] = line;
				line = malloc(sizeof(char) * 256);
				i = 0;
				line[i] = '\0';
			}
			i = 0;
		}else{
			line[i++] = c;
			line[i] = '\0';
		}
	}

	fclose(read);
}

Results* getResults(char* fileName, char allocatorType, int numRegs){
	Results* results = malloc(sizeof(Results));
	results->numResultLines = 0;
	results->results = malloc(sizeof(char*) * 128);	// A maximum of 128 result lines
	results->fileName = fileName;
	results->allocator = allocatorType;
	results->executionTime = 0;
	results->numRegs = numRegs;

	fetchResults(results);

	Results* real = malloc(sizeof(Results));
	real->results = malloc(sizeof(char*) * 128);
	real->numResultLines = 0;
	real->fileName = fileName;
	real->allocator = -1;
	real->numRegs = numRegs;
	real->executionTime = 0;

	fetchResults(real);

	results->resultsCorrect = areResultsEqual(results, real);
	
	return results;
}

char* allocatorTypeToText(char type){
	switch(type){
		case 'b': return "Bottom Up";
		case 's': return "Top Down EAC";
		case 't': return "Top Down Class";
		default: return "Err";
	}
}

int main(int argc, char** argv){

	int i, j, k;
	int numRegs;
	char* fileName;
	char testSymbol;

	FILE* file = fopen("output", "w");
	fprintf(file, "%s, %s, %s, %s, %s, %s\n", "Allocator", "File Name", "Number Registers", "Cycles", "Allocator Time", "Correct");
//	fprintf(file, "%s, %s, %s, %s\n", "File Name", "Time @ 5 regs", "Time @ 10 regs", "Time @ 20 regs");
	for(k = 0; k < NUM_TEST_FILES; k ++){

			for(i = 0; i < NUM_TESTING_SYMBOLS; i ++){
			//	if(testingSymbols[i] != 't') continue;

	for(j = 0; j < NUM_TEST_REGS; j ++){
				Results* results = getResults(files[k], testingSymbols[i], numTestRegisters[j]);

//				printf("Testing allocator %c ON %s WITH %d registers\n", testingSymbols[i], files[k], numTestRegisters[j]);
				fprintf(file, "%s, %s, %d, %d, %f, %d\n", allocatorTypeToText(results->allocator), results->fileName, results->numRegs, results->numCycles, results->executionTime, results->resultsCorrect);
//				return 0;
			}
//				fprintf(file, "%s, %f, %f, %f\n", files[k], num5, num10, num20);	
		}

	}

	fclose(file);

	return 0;

}
