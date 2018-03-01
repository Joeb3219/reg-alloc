#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_TEST_FILES 5
#define NUM_TEST_REGS 3
#define NUM_TESTING_SYMBOLS 3

char *files[NUM_TEST_FILES] = {"../samples/test1.i", 	"../samples/test3.i", 	"../samples/test4.i", 	"../samples/my_test.i", "../samples/report5.i"};
char *fileArgs[NUM_TEST_FILES] = {"-i 1024 1", 		"-i 1024 1 1",		"-i 1024 1 1",		"",			"-i 1024 1"};
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
	if(fork() == 0){
		printf("Getting results of %s %s\n", results->fileName, getFileArgs(results->fileName));
                execl("/home/joeb3219/code/cs415/register/sim", "sim", getFileArgs(results->fileName), "<", results->fileName, ">", "results", NULL);

		FILE* read = fopen("results", "r");

		while( (c = fgetc(read)) != EOF){
			printf("%c\n", c);
		}

		fclose(read);

                exit(1);
        }else{
                // We wait for this GCC call to finish.
                wait(NULL);
        }
}

Results* getResults(char* fileName, char allocatorType, int numRegs){
	Results* results = malloc(sizeof(Results));
	results->numResultLines = 0;
	results->results = malloc(sizeof(char*) * 128);	// A maximum of 128 result lines
	results->fileName = fileName;
	results->allocator = allocatorType;

	fetchResults(results);

	Results* real = malloc(sizeof(Results));
	real->results = malloc(sizeof(char*) * 128);
	real->numResultLines = 0;
	real->fileName = fileName;
	real->allocator = -1;

	fetchResults(real);

	results->resultsCorrect = areResultsEqual(results, real);

	return results;
}

int main(int argc, char** argv){

	int i, j, k;
	int numRegs;
	char* fileName;
	char testSymbol;

	for(i = 0; i < NUM_TESTING_SYMBOLS; i ++){

		for(j = 0; j < NUM_TEST_REGS; j ++){

			for(k = 0; k < NUM_TEST_FILES; k ++){
				Results* results = getResults(files[k], testingSymbols[i], numTestRegisters[j]);
//				printf("Testing allocator %c ON %s WITH %d registers\n", testingSymbols[i], files[k], numTestRegisters[j]);

			}

		}

	}

	return 0;

}
