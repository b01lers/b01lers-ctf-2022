#!/usr/bin/env python


import sys
import os
import random
import struct


MEMSIZE = 1024*1024
NREGS = 10
MAXTEXT = 2000
MAXINST = 55 #mine is 52 # I am almost sure it is possible to make it 1 or 2 instructions smaller


class Instruction:
    op = None
    imm = 0
    o1 = None
    o2 = None
    dsp = None
    target = None
    memory_address = 0
    clock = 0


    def __init__(self, tstr):
        def parse_immediate(tt):
            if tt.startswith("0x"):
                try:
                    v = int(tt, 16)
                except ValueError:
                    assert False
            else:
                try:
                    v = int(tt)
                except ValueError:
                    assert False
            assert v>=0
            assert v<pow(2,32)
            return v

        def parse_target(tt):
            try:
                v = int(tt)
            except ValueError:
                assert False
            assert v>-1000
            assert v<1000
            return v

        def parse_register(tt):
            assert len(tt) == 2
            assert tt[0] == "r"
            try:
                v = int(tt[1])
            except ValueError:
                assert False
            assert v>=0
            assert v<NREGS
            return v

        def parse_memory(tt):
            try:
                v = int(tt)
            except ValueError:
                assert False
            assert v>=0
            assert v<MEMSIZE
            return v

        assert len(tstr)<100
        sstr = tstr.split()
        assert len(sstr)>=1
        assert len(sstr)<=4

        if len(sstr) == 1:
            t_op = sstr[0]
            assert t_op in ["halt", "time", "magic", "reset"]
            self.op = t_op

        elif len(sstr) == 2:
            t_op, t_1 = sstr
            assert t_op in ["jmp", "jmpz"]
            self.op = t_op

            if self.op == "jmp":
                self.target = parse_target(t_1)
            elif self.op == "jmpz":
                self.target = parse_target(t_1)
            else:
                assert False

        elif len(sstr) == 3:
            t_op, t_1, t_2 = sstr
            assert t_op in ["mov", "movc", "jmpg", "add", "sub", "mul", "and", "or", "xor"]
            self.op = t_op

            if self.op == "mov":
                self.o1 = parse_register(t_1)
                self.o2 = parse_register(t_2)

            elif self.op in ["add", "sub", "mul", "and", "or", "xor"]:
                self.o1 = parse_register(t_1)
                self.o2 = parse_register(t_2)

            elif self.op == "movc":
                self.o1 = parse_register(t_1)
                self.imm = parse_immediate(t_2)

            elif self.op == "jmpg":
                self.target = parse_target(t_2)
                self.o1 = parse_register(t_1)

            else:
                assert False

        elif len(sstr) == 4:
            t_op, t_1, t_2, t_3 = sstr
            assert t_op in ["movfrom", "movto"]
            self.op = t_op

            if self.op == "movfrom":
                self.o1 = parse_register(t_1)
                self.memory_address = parse_memory(t_2)
                self.dsp = parse_register(t_3)

            elif self.op == "movto":
                self.o1 = parse_register(t_1)
                self.memory_address = parse_memory(t_2)
                self.dsp = parse_register(t_3)

            else:
                assert False

        else:
            assert False


    def pprint(self):
        tstr = "%s %s %s %s %s %s" % \
            (self.op, 
            "None" if self.o1==None else "r%d"%self.o1,
            "None" if self.o2==None else "r%d"%self.o2,
            hex(self.imm), "None" if self.target==None else self.target, self.memory_address)
        return tstr


class Cpu:
    ipointer = 0
    instructions = None
    registers = None
    memory = None
    clock = 0
    cache = None
    execution_level = 0
    secret_values = None


    def __init__(self):
        self.instructions = []
        self.cache = {}
        self.secret_values = (random.randint(1,4200000000), random.randint(1,4200000000) , random.randint(1,4200000000), random.randint(1,4200000000))

        self.reset()


    def reset(self):
        self.ipointer = 0
        #self.instructions = []
        self.registers = [0 for r in range(NREGS)]
        self.memory = [0 for _ in range(MEMSIZE)]
        self.clock = 0
        for k in self.cache.keys():
            self.cache[k] = 0
        self.execution_level += 1
        #secret_values


    def load_instructions(self, tt):
        for line in tt.split("\n"):
            if "#" in line:
                line = line.split("#")[0]
            line = line.strip()
            if not line:
                continue
            self.instructions.append(Instruction(line))
            assert len(self.instructions) <= MAXINST

    def run(self, debug=0):
        if debug>0: print(self.pprint())
        ins = self.instructions[0]
        for i,v in enumerate(self.secret_values):
            self.memory[i] = v

        while (self.ipointer>=0 and self.ipointer<len(self.instructions) and self.execution_level<4 and self.clock<20000):
            ins = self.instructions[self.ipointer]

            if debug>0: print("==>", ins.pprint())
            self.execute(ins)
            if debug>0: print(self.pprint(debug=debug))
            #print("===", self.ipointer, len(self.instructions), self.ipointer>=0 and self.ipointer<len(self.instructions))

        if debug>0: print("Terminated! (at %d)" % self.ipointer)
        if debug>0: print("State:")
        if debug>0: print(self.pprint())

    def execute(self, ins):
        self.clock += 1

        if ins.op == "movc":
            self.registers[ins.o1] = ins.imm
            self.ipointer += 1

        elif ins.op == "magic":
            if self.execution_level == 2:
                #print(self.registers, self.secret_values, self.registers[0:2], self.registers[0:2] == self.secret_values)
                if tuple(self.registers[0:4]) == self.secret_values:
                    #print("===")
                    with open("flag.txt", "rb") as fp:
                        cc = fp.read()
                    cc = cc.strip()
                    cc = cc.ljust(len(self.registers)*4, b"\x00")
                    for i in range(len(self.registers)):
                        self.registers[i] = struct.unpack("<I", cc[i*4:(i+1)*4])[0]

            self.ipointer += 1

        elif ins.op == "reset":
            self.reset()

        elif ins.op == "halt":
            self.ipointer = len(self.instructions)

        elif ins.op == "time":
            self.registers[0] = self.clock
            self.ipointer += 1

        elif ins.op == "jmp":
            nt = self.ipointer + ins.target
            assert nt >=0 
            assert nt < len(self.instructions)
            self.ipointer = nt

        elif ins.op == "jmpz":
            if self.registers[0] == 0:
                nt = self.ipointer + ins.target
                assert nt >=0 
                assert nt < len(self.instructions)
                self.ipointer = nt
            else:
                self.ipointer += 1

        elif ins.op == "jmpg":
            if self.registers[0] > self.registers[ins.o1]:
                nt = self.ipointer + ins.target
                assert nt >=0 
                assert nt < len(self.instructions)
                self.ipointer = nt
            else:
                self.ipointer += 1

        elif ins.op == "mov":
            self.registers[ins.o1] = self.registers[ins.o2]
            self.ipointer += 1

        # ALU
        elif ins.op == "sub":
            v = self.registers[ins.o1] - self.registers[ins.o2]
            self.registers[ins.o1] = (v & 0xffffffff)
            self.ipointer += 1

        elif ins.op == "add":
            v = self.registers[ins.o1] + self.registers[ins.o2]
            self.registers[ins.o1] = (v & 0xffffffff)
            self.ipointer += 1

        elif ins.op == "mul":
            v = self.registers[ins.o1] * self.registers[ins.o2]
            self.registers[ins.o1] = (v & 0xffffffff)
            self.ipointer += 1

        elif ins.op == "and":
            v = self.registers[ins.o1] & self.registers[ins.o2]
            self.registers[ins.o1] = (v & 0xffffffff)
            self.ipointer += 1

        elif ins.op == "or":
            v = self.registers[ins.o1] | self.registers[ins.o2]
            self.registers[ins.o1] = (v & 0xffffffff)
            self.ipointer += 1

        elif ins.op == "xor":
            v = self.registers[ins.o1] ^ self.registers[ins.o2]
            self.registers[ins.o1] = (v & 0xffffffff)
            self.ipointer += 1

        # MEMORY
        elif ins.op == "movfrom":
            mem_location = ins.memory_address + self.registers[ins.dsp]
            mem_location = mem_location % len(self.memory)

            #print(mem_location, self.cache)
            if mem_location in self.cache:
                #print("-cache")
                v = self.cache[mem_location]
                v = (v & 0xffffffff)
                self.registers[ins.o1] = v
                self.ipointer += 1
            else:
                #print("-nocache")
                v = self.memory[mem_location]
                self.cache[mem_location] = v
                self.execute(ins)

        elif ins.op == "movto":
            mem_location = ins.memory_address + self.registers[ins.dsp]
            mem_location = mem_location % len(self.memory)

            if mem_location in self.cache:
                del self.cache[mem_location]
            v = (self.registers[ins.o1] & 0xffffffff)
            self.memory[mem_location] = v
            self.ipointer += 1

        else:
            assert False

        return 

    def pprint(self, debug=0):
        tstr = ""
        tstr += "%d> "%self.ipointer
        tstr += "[%d] "%self.clock
        tstrl = []
        for i,r in enumerate(self.registers):
            tstrl.append("r%d=%d"%(i,r))
        tstr += ",".join(tstrl)
        if debug>1:
            tstr += "\nM->"
            vv = []
            for i,v in enumerate(self.memory):
                if v!=0:
                    vv.append("%d:%d"%(i,v))
            tstr += ",".join(vv)
            tstr += "\nC->"
            tstr += repr(self.cache)
        return tstr






def main():
    print("Welcome to a very fast VM!")
    print("Give me your instructions.")
    print("To terminate, send 3 consecutive empty lines.")


    instructions = ""
    wlc = 0
    while True:
        line = input()
        #print(repr(line))
        if not line.strip():
            wlc += 1
        else:
            wlc = 0
        instructions += line + "\n"
        if wlc >= 3 or len(instructions) > MAXTEXT:
            break

    c = Cpu()
    print("Parsing...")
    c.load_instructions(instructions)
    print("Running...")
    c.run()

    print("Done!")
    print("Registers: " + repr(c.registers))
    print("Goodbye.")


if __name__ == "__main__":
    sys.exit(main())


#~/pypy3.6/bin/pypy3 ./cpu.py test
#~/pypy3.6/bin/pypy3 ./cpu.py exploit


