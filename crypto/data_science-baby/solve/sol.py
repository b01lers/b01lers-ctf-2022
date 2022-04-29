from pwn import *
from math import sqrt


def solveOne(r):
   in1 = r.recvuntil(b"[a,b]:");
   in1 = in1.strip().split(b"\n")
   A = float( in1[-5].strip()[2:] )
   B = float( in1[-4].strip()[2:] )
   C = float( in1[-3].strip()[2:] )
   print(f"A,B,C = {A} {B} {C}")
   xlo = (-B + sqrt(B * B - 4. * A * C)) / (2. * A)
   xmid = -B / (2. * A)
   delta = xmid - xlo
   # stretch [a,b] interval as wide as we can while keeping xlo almost exactly on a grid site
   x = xlo + 1e-6
   MAX, N = 399.9999999, 1000
   i = int((x + MAX) / (2. * MAX) * N  + 0.5)
   dlo = (x + MAX) / i
   dhi = (MAX - x) / (N - i)
   d = min(dlo, dhi)
   a = x - d * i
   b = a + N * d
   print(f"a={a} b={b} delta={delta}") 
   #
   r.send(f"{a},{b}\n".encode("ascii"))
   return r.recvall()


def solve(local):
   iter = 0
   while True:
      iter += 1
      print(f"#TRIES: {iter}\n")
      r = process(["./data_science-baby-O2", "2.51"])   if local   else   remote("localhost", 25002)
      flag = solveOne(r)
      r.close()
      print(f"RES: {flag}")
      if b"bctf" in flag: break


#solve(True)
solve(False)
