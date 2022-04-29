#ifndef TM_H
#define TM_H

#define NUM_STATES 100
#define TAPE_SIZE 256
#define ALPH_SIZE 256

struct Node **states;
char *tape;
char *tape_base;
char *inputTape;
char *inputTape_base;

struct Node {
	int (*actions[ALPH_SIZE])();
	int gotos[ALPH_SIZE];
};

#endif
