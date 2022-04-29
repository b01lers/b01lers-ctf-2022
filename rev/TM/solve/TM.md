The basic concept of a Turing Machine is that it looks at a single position of a tape, does something, then maybe moves the place it reads to something different,
then looks at this new position, does something, etc. etc. The amazing thing is that this is supposed to be able to simulate everything that is theoretically computable!
(of course, this isn't really a formal definition or anything but basically the gist of it)

The full construction of the TM is in the source code (which has some comments on what each state is doing, where it's going, etc.)
The struct used to implement this is 
```
struct Node {
	int (*actions[ALPH_SIZE])();
	int gotos[ALPH_SIZE];
};
```
and the code used to "run" the TM is
```
struct Node * cur = states[0];
while (running) {
		next = states[cur->gotos[*tape]];
		running = (cur->actions[*tape])();
		cur = next;
	}
```
Basically, each node has a list of actions (function pointers), one for each possible value of the tape (in our case, 256 since the tape head is reading a whole byte)
and a list of next nodes (also 256 items). We look at the current tape value `x`, execute the function on the current state associated with `x`, and go to the next state (determined by `x`).
The function pointers in Node return 1 if the TM should keep running and return 0 if the TM should stop/halt. 

Reading through `tm.c` should basically tell you what it's doing, but to keep it short here: 
for every byte in the input, it adds 10 if the byte value is even, then XORs with 69 (heh), then compares with a fixed string.
This comparison happens through a chain of states that either go to a REJECT state if the byte doesn't match or go to the next non-rejecting state, 
and it might be worth noting that it's checked in reverse order so I didn't have to spend work rewinding the tape to the beginning of the input correctly.
