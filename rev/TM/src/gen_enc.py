flag = "bctf{tur1ng_m4_C_1n3}"
print(len(flag))
enc = ""
for i in flag:
	c = ord(i)
	if c % 2 == 0:
		c += 10
	c = c ^ 69
	enc += chr(c)
print(enc.encode("utf-8"))
print(enc[::-1].encode("utf-8"))
