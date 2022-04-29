from pwn import *


def runSolver(Astr, Bstr, Cstr):
   r = process(["./sol-O2", Astr, Bstr, Cstr])
   a, b = None, None
   while True:
     in1 = r.recvuntil(b"\n")
     print(in1)
     if b"unsuitable" in in1:            # unsolvable case
        b = b"bad"
        break
     if b"sol:" in in1: 
        in1 = in1.strip().split()
        a = in1[1][2:]
        b = in1[2][2:]
        if b"1e+100" in a:  a = None     # solver failed
        break
   r.close()
   return a, b   # best a,b

def solveOne(r):
   in1 = r.recvuntil(b"[a,b]:");
   in1 = in1.strip().split(b"\n")
   A = in1[-5].strip()[2:]
   B = in1[-4].strip()[2:]
   C = in1[-3].strip()[2:]
   a, b = runSolver(A, B, C)
   print(a, b)
   if a == None: return b
   r.send(a + b"," + b + b"\n")
   return r.recvall()


def solve(local):
   iter, bad, fail = 0, 0, 0
   while True:
      iter += 1
      print(f"#TRIES: {iter}, UNSUITABLE: {bad}, FAIL: {fail}\n")
      r = process(["./data_science-O2", "0."])   if local   else   remote("localhost", 25001)
      flag = solveOne(r)
      r.close()
      if flag == b"bad":  bad += 1
      elif flag == b"1e+100": fail += 1
      else:
         print(f"RES: {flag}")
         if b"bctf" in flag: break


#solve(True)
solve(False)
