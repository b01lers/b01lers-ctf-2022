from z3 import *
flag = IntVector("f", 22)
counter = IntVector("f",4)
s = Solver()
for i in range(0, 22):
	s.add(flag[i] >= 0x21)
	s.add(flag[i] <= 0x7e)
s.add(flag[0] + flag[1]+flag[2]+flag[3] == 0x115)
s.add(flag[4] * flag[5] +flag[6] + flag[7] == 0x1337)
s.add(flag[8]  +  flag[9] >= 100)
s.add(flag[10] / flag[11] == 2)
s.add(flag[12] * flag[13] > 1000)
s.add((flag[14] +flag[15])*5 > 200)
s.add(flag[16] - flag[17] == 42)
s.add(flag[18] - flag[19] == 2)
s.add(flag[20] * flag[21] >= 900 )
if(s.check() != unsat):
    mod = s.model()
    for i in range(22):
        x = mod[flag[i]].as_long()
        print (chr(x),end='')
else:
    print("Damn, no sol...")
