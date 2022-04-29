import ctypes
import sys
from pwn import *

'''
We never seed rand(), so the values that rand() spits out are always the same.
Mimic this easily with ctypes
To get to 1000 coins, we need to guess correctly 90 times
'''

libc = ctypes.CDLL('libc.so.6')
full = b""
for i in range(90):
	full += bytes([libc.rand() % 26 + ord('a') for j in range(4)])

p = remote("localhost", 7777)
p.send(full)
p.interactive()
