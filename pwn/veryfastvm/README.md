This is a Python challenge. 

I categorized it as a pwn, but it has no memory corruption.
In a sense, this challenge could also be categorized as rev.
The main goal of the player is first to reverse how the provided Python VM works, and then figure out how to make it print out the flag.

The VM has "standard" instructions, such as `mov`, `add`, `jmp`, `or`, ...
It also has registers and memory.
The player can provide arbitrary code to the VM, which executes it.
When the execution terminates, the VM prints out the values of its 10 registers.

When started 4 secret values are stored in memory at address 0.
When started the execution_level is 0.
Every time the `reset` instruction is called, the execution_level is incremented by 1.
The `reset` instruction also sets to zero all the registers and all the memory.
If the execution_level is 1, a special instruction called `magic` is enabled.
The `magic` instruction returns the content of flag.txt if, when called, the first 4 registers contain the 4 secret values mentioned above.
However, since the `magic` instruction can only be called after a `reset` instruction, it is hard to write a program that "remembers" the 4 secret values after a `reset` instruction.

The trick is to use the cache.
Every time a specific memory address is read, it is placed in cache.
When the memory is in cache, it only requires 1 clock cycle to be accessed, otherwise it requires 2 clock cycles.
The instruction `time` allows the player to know the current clock value.
The cache is implemented as a Python dictionary.
The `reset` instruction sets every entry (i.e., every value) in the cache equal to 0, but it does not remove the address from the cache (i.e., it does not delete the keys from the cache dictionary).
Hence, after a `reset` instruction, the provided code can understand if a specific address has been stored in the cache, by checking how much the clock incremented after accessing that specific address.

By using this idea, the exploit works in three steps.
First of all, different memory accesses are performed based on the content of the 4 secret values.
Then, the `reset` instruction is called.
Finally, the `time` instruction is used to determine if specific memory addresses were set during the first phase and recover the secret value.
Implementing this code is not trivial, given all the limitations the implemented VM language have.
The solution (i.e., the code for the Python VM that is making it printing out the flag) is in `solve/exploit`.
The flag is not printed out directly, but it is stored in the VM's registers.
The values of those registered is printed out when the execution of the provided code terminates.
`solve/explot.py` is just a script sending the code in `solve/exploit` to an arbitrary `IP/port` and parsing the returned value.

The cpu.py code provided to the players is slightly obfuscated.
A script (build.py) takes cpu_original.py and removes all the comments and whitelines.
Then, it renames most of the variables.
To re-build and re-test cpu.py, run `src/build_and_test.sh`. 
This script also takes care of moving files from `src/` to the appropriate folders.

