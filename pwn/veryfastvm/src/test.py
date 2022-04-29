#/usr/bin/env python

import sys
import os
import struct
import time
import subprocess
from cpu import Cpu

import cpu; print("==>", cpu.__file__)


def test1():
    print("TEST1 started")
    with open("p1") as fp:
        iii = fp.read()

    c = Cpu()
    c.load_instructions(iii)
    c.run()

    assert c.X00 == 1
    assert c.X01 == [8,16,17,33,2178,0,0,0,0,0]
    print("TEST1 succeded")


def test2():
    print("TEST2 started")
    with open("p2") as fp:
        iii = fp.read()

    c = Cpu()
    c.load_instructions(iii)
    c.run()

    assert c.X01 == [0,0,162,0,0,0,0,0,3,1]
    print("TEST2 succeded")


def test3():
    print("TEST3 started")
    with open("p3") as fp:
        iii = fp.read()

    c = Cpu()
    c.load_instructions(iii)
    c.run()

    assert c.X01 == [2,4294967291,7,2,4294967295,2,1,3,24,1]
    print("TEST3 succeded")


def test4():
    print("TEST4 started")
    with open("p4") as fp:
        iii = fp.read()

    c = Cpu()
    c.load_instructions(iii)
    c.run()

    print(c.pprint(debug=2))
    assert c.X02 == 21-((3-1)*3)
    assert c.X01 == [20-((3-1)*3),6,4,0,0,77,77,0,2,2]
    assert c.memory[13] == 4
    assert c.memory[12] == 77
    assert any([v for i,v in enumerate(c.memory) if (i!=13 and i!=12 and i>=4)])==False
    assert c.X03 == {18:0,13:4,12:77}
    print("TEST4 succeded")


def test5():
    print("TEST5 started")
    with open("p5") as fp:
        iii = fp.read()

    c = Cpu()
    c.load_instructions(iii)
    c.run(debug=2)

    print(c.pprint(debug=2))
    assert c.X00 == 2
    assert c.X01 == [0,0,33,0,0,0,0,0,0,0]
    assert any([v for v in c.memory]) == False
    assert c.X03 == {0:0, 2:0, 1:0}
    assert c.X02 == 8-(3-1)
    print("TEST5 succeded")


def test6():
    print("TEST6 started")
    with open("p6") as fp:
        iii = fp.read()

    c = Cpu()
    c.load_instructions(iii)
    c.run(debug=2)

    print(c.pprint(debug=2))
    assert c.X01 == [4000000000,4000000001,4000000002,4000000003,0,0,0,0,0,0]
    assert any([v for v in c.memory]) == False

    with open("p6") as fp:
        iii = fp.read()

    c = Cpu()
    c.load_instructions(iii)
    fixed_secret_values = (4000000000,4000000001, 4000000002, 4000000003)
    c.X04 = fixed_secret_values
    c.run(debug=2)

    print(c.pprint(debug=2))
    for i in range(2):
        assert c.X01[i] != 0
    for i,v in enumerate(fixed_secret_values):
        c.X01[i] != v
    assert any([v for v in c.memory]) == False
    tstr = b""
    for v in c.X01:
        tstr += struct.pack("<I", v)
    flag = tstr.split(b"\X0")[0].decode("utf-8")
    print(repr(c.X01))
    print("FLAG:", flag)
    assert "{" in flag
    assert "}" in flag
    print("TEST6 succeded")


def test7():
    print("TEST7 started")
    iii = "jmp 0\n\n\n"
    c = Cpu()
    c.load_instructions(iii)
    t = time.time()
    c.run()
    print("=>", c.X00, c.X01, c.X02, time.time()-t) #0.023 * 5 * 5 ~ 1s
    assert c.X02 == 20000

    iii = "movc r0 0\nreset\nmovc r0 1\nreset\nmovc r0 2\nreset\nmovc r0 3\nreset\nmovc r0 4\nreset\nmovc r0 5\nreset\nmovc r0 6\n\n\n\n"
    c = Cpu()
    c.load_instructions(iii)
    t = time.time()
    c.run()
    print("=>", c.X00, c.X01, c.X02, time.time()-t)
    assert c.X00 == 4
    assert all(r==0 for r in c.X01)
    print("TEST7 end")


def exploit():
    t = time.time()
    with open("exploit") as fp:
        iii = fp.read()

    print("START")
    c = Cpu()
    c.load_instructions(iii)
    #c.run(debug=2)
    c.run()

    tstr = b""
    for v in c.X01:
        tstr += struct.pack("<I", v)
    flag = tstr.split(b"\X0")[0].decode("utf-8")
    print("=>", c.X00, c.X01, c.X02)
    print("FLAG:", flag)
    assert "{" in flag
    assert "}" in flag
    assert c.X01[9] == 0

    print(time.time()-t) #<0.1
    print("END")


def test_as_subprocess():
    PYTHON3 = "python3"
    PIPE = subprocess.PIPE

    files = ["p1", "p2", "p3", "p4", "p5", "p6", "exploit"]
    for fname in files:
        print("===", fname)
        with open(fname, "rb") as fp:
            cc = fp.read()
        print(repr(cc))

        p = subprocess.Popen([PYTHON3 + " -u ./cpu.py"], stdin=PIPE, stdout=PIPE, stderr=PIPE, shell=True)
        res = p.communicate(input=cc)
        r = p.wait()
        print(repr(res))
        assert r==0
        assert res[1] == b""
        assert res[0].endswith(b"\nGoodbye.\n")

        if fname == "p1":
            assert b"Registers: [8, 16, 17, 33, 2178, 0, 0, 0, 0, 0]" in res[0]
        elif fname == "p2":
            assert b"Registers: [0, 0, 162, 0, 0, 0, 0, 0, 3, 1]" in res[0]
        elif fname == "p3":
            assert b"Registers: [2, 4294967291, 7, 2, 4294967295, 2, 1, 3, 24, 1]" in res[0]
        elif fname == "p4":
            assert b"Registers: [14, 6, 4, 0, 0, 77, 77, 0, 2, 2]" in res[0]
        elif fname == "p5":
            assert b"Registers: [0, 0, 33, 0, 0, 0, 0, 0, 0, 0]" in res[0]
        elif fname == "p6":
            assert b"Registers: [4000000000, 4000000001, 4000000002, 4000000003, 0, 0, 0, 0, 0, 0]" in res[0]
        elif fname == "exploit":
            assert b"Registers" in res[0]
            for l in res[0].split(b"\n"):
                if b"Registers" in l:
                    break
            registers = [int(v) for v in l.split(b"[")[1].split(b"]")[0].split(b",")]
            tstr = b""
            for v in registers:
                tstr += struct.pack("<I", v)
            flag = tstr.split(b"\X0")[0].decode("utf-8")
            print("FLAG:", flag)
            assert "{" in flag
            assert "}" in flag
            assert registers[9] == 0



if __name__ == "__main__":
    test1()
    test2()
    test3()
    test4()
    test5()
    test6()
    test7()

    exploit()


    test_as_subprocess()




#python3 ./cpu.py
#python3 ./test.py



