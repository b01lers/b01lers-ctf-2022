# this can be solved via dictionary attack
#
# use a huge word list, e.g.: https://www.keithv.com/software/wlist/


import gzip
from Crypto.Hash import SHA256
from pwn import *


def hash(s):
   h = SHA256.new()
   h.update(s.lower())
   return h.hexdigest()

def guess1(r, w):
   r.recvuntil(b"choose:")
   r.send( f"1\n{w}\n".encode() )

   in1 = r.recvuntil(b"\n")

   ret = in1.strip().split()[-1]
   ret = ret.replace(b"\x1b[43m", b"Y")
   ret = ret.replace(b"\x1b[40m", b"B")
   ret = ret.replace(b"\x1b[42m", b"G")
   ret = ret.replace(b"\x1b[0m", b"")
   return ret

   
def doOne(r, guesses):
   pool = hashes.copy()
   for w in guesses:
      print(f"#GUESS: {w}")
      score = guess1(r, w).decode()
      #print(f"#->{score}")

      greens = []
      for k in range(64):
         if score[2 * k] == 'G': greens.append( (k, score[2 * k + 1]) )
      print(f"#-> {greens}" )

      newPool = []
      for (w,h) in pool:
         for pair in greens:
            if h[pair[0]] != pair[1]:   break
         else:   newPool.append( (w, h) )
      if len(newPool) < 10:   print(f"NEW POOL: {newPool}")
      else:   print( f"NEW POOL: ({len(newPool)} words)" )

      pool = newPool
      if len(pool) == 1:
         result = guess1(r, pool[0][0].decode() )
         return result.count(b"G") == 64
      elif len(pool) == 0:   return False

def doAll(r, Nrounds):
   #r = process(['python3', 'SHArdle.py'])
   r = remote("localhost", 25000)

   for i in range(Nrounds):
      success = doOne(r, ["hello", "baby", "girl"] )
      if not success:   break 
   else:
      print( r.recvall() )
      return True

   r.close()
   return False
         


#words = open("Ulysses_4300-0.txt", "rb").read().split()
#words = open("english.txt", "rb").read().split()
words = gzip.open("wlist_match1.txt.gz", "rb").read().split()
wordDict = {}
for i in range(len(words)):
   w0 = words[i].lower()
   w = b""
   for c in w0:
      if 0x61 <= c <= 0x7a: w += bytes([c])
   if wordDict.get(w) == None: wordDict[w] = 1
words = [w  for w in wordDict]

print( f"# total {len(words)} words" )

hashes = [ (w, hash(w))  for w in words ]

count = 0
while True:
   count += 1
   print( f"###TRY {count}" )
   success = doAll(hashes, 2)
   if success:
      print( f"total tries: {count}" )
      break

