#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tm.h"

#define set_all_gotos(x) for(i=0;i<ALPH_SIZE;i++)state->gotos[i]=x;
#define set_all_actions(x) for(i=0;i<ALPH_SIZE;i++)state->actions[i]=x;

int copy_input_to_tape() {
	*tape = *inputTape;
	inputTape++;
	return 1;
}

int print_input() {
	printf("input tape contents: %s", inputTape_base);
	return 1;
}

int print_tape() {
	printf("work tape contents : %s", tape_base);
	return 1;
}

int add_10() { *tape += 10; return 1; }
int reject() { printf("Input rejected.\n"); return 0; }
int accept() { printf("Input accepted.\n"); return 0; }
int nop() { return 1; }
int xor_69_inc_tape() { *tape ^= 69; tape++; return 1; }
int tm_exit() { return 0; }
int inc_tape() { tape++; return 1; }
int dec_tape() { tape--; return 1; }
int dec_tape_twice() { tape--; tape--; return 1; }

void setup()
{
	// alloc the initial stuff
	states = malloc(NUM_STATES * sizeof(struct Node));
	memset(states, 0, NUM_STATES * sizeof(struct Node));
	struct Node * state;
	int i = 0;

	// state 0: empty placeholder
	state = malloc(sizeof(struct Node));
	set_all_actions(nop);
	set_all_gotos(1);
	states[0] = state;

	// state 1: copy cur input byte to worktape
	// -> 2
	state = malloc(sizeof(struct Node));
	set_all_actions(copy_input_to_tape);
	set_all_gotos(2);
	states[1] = state;

	// state 2: add 10 if tapeHead if input byte is even
	// if tapeHead is null byte, we should start doing checks
	// -> 3 if tape is not nullbyte
	// -> 4 if tape is nullbyte
	state = malloc(sizeof(struct Node));
	set_all_actions(i%2 ? nop : add_10);
	set_all_gotos(3);
	state->gotos[0] = 4;
	states[2] = state;

	// state 3: xor tape byte by 69 (heh), then go back to state 1 to write down next input byte
	// -> 1
	state = malloc(sizeof(struct Node));
	set_all_actions(xor_69_inc_tape);
	set_all_gotos(1);
	states[3] = state;

	// state 4: decrement tapeHead so we start at the last
	// -> 7
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape_twice);  // dec tape twice to skip over the newline
	set_all_gotos(7);
	states[4] = state;

	// state 5: fail...
	state = malloc(sizeof(struct Node));
	set_all_actions(reject);
	states[5] = state;

	// state 6: success!
	state = malloc(sizeof(struct Node));
	set_all_actions(accept);
	states[6] = state;

	// states 7-end: check chars
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[56] = 8;
	states[7] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[118] = 9;
	states[8] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[61] = 10;
	states[9] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[116] = 11;
	states[10] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[26] = 12;
	states[11] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[6] = 13;
	states[12] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[26] = 14;
	states[13] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[123] = 15;
	states[14] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[40] = 16;
	states[15] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[26] = 17;
	states[16] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[34] = 18;
	states[17] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[61] = 19;
	states[18] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[116] = 20;
	states[19] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[57] = 21;
	states[20] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[48] = 22;
	states[21] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[59] = 23;
	states[22] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[62] = 24;
	states[23] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[53] = 25;
	states[24] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[59] = 26;
	states[25] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[38] = 27;
	states[26] = state;
	state = malloc(sizeof(struct Node));
	set_all_actions(dec_tape);
	set_all_gotos(5);
	state->gotos[41] = 6;  // go to accept state if the final check passes
	states[27] = state;
}

int main()
{
	// create TM
	setup();

	// create work tape
	tape_base = malloc(TAPE_SIZE);
	tape = tape_base;
	memset(tape, 0, TAPE_SIZE);

	// read input tape
	inputTape_base = malloc(TAPE_SIZE);
	inputTape = inputTape_base;
	int len = read(0, inputTape, TAPE_SIZE);	
	inputTape[len] = 0;

	// let's just say states[0] is always the start state
	struct Node * cur = states[0];
	struct Node * next;
	int running = 1;
	while (running) {
		next = states[cur->gotos[*tape]];
		running = (cur->actions[*tape])();
		cur = next;
	}

	// clean up
	for (int i = 0; i < NUM_STATES; i++) free(states[i]);
	free(tape_base);
	free(inputTape_base);
	free(states);
}
