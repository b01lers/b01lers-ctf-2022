from SHArdle_helpers import FLAG, generate, valid

from Crypto.Hash import SHA256
from Crypto.Random.random import randint
from collections import Counter


def hash(s):
   h = SHA256.new()
   h.update(s)
   return h.hexdigest()

def green(c):   return '\033[42m' + c + '\033[0m'
def gray(c):    return '\033[40m' + c + '\033[0m'
def yellow(c):  return '\033[43m' + c + '\033[0m'

def compare(s, s0):
   n = len(s0)
   assert len(s) == n
   # get greens
   ans = [None]*n
   rest0 = Counter()
   for i in range(n):
      if s[i] == s0[i]: ans[i] = green(s[i])
      else: rest0.update(s0[i])
   # color rest
   for i in range(n):
      if ans[i] != None: continue
      cnt = rest0.get(s[i])
      if cnt == None or cnt == 0: ans[i] = gray(s[i])
      else:
         rest0[s[i]] -= 1
         ans[i] = yellow(s[i])
   return "".join(ans)


def hashAndCompare(s, s0):
   h = hash(s)
   h0 = hash(s0)
   return compare(h, h0)


## GAME


class Game:

   def __init__(self):
      self.rounds = 2
      self.secrets = [ generate(randint)   for i in range(self.rounds) ]
      self.start()

   def menu(self, round):
      print(f"1 - guess secret {round}")
      print( "2 - exit")
      print( "choose: ", end = "")

   def start(self):

      print( "I thought of some secrets, can you guess those?\n" )

      guesses = 15
      round = 0
      while guesses > 0:
         try:
            self.menu(round + 1)
            s = int( input("").strip() )
            if s == 2:  exit(0)
            elif s == 1:
               s = input("your guess: ").strip().encode().lower()
               if not valid(s):  print("INVALID GUESS")
               else:
                  print(f"score: {hashAndCompare(s, self.secrets[round])}")
                  if s == self.secrets[round]:
                     round += 1
                     print(f"BINGO!! {self.rounds - round} more to go")
                     if round == self.rounds:
                        print(f"Here is the flag: {FLAG}")
                        exit(0)
                  guesses -= 1
         except ValueError:
            print("invalid choice")
      print("No more guesses - goodbye!")


if __name__ == "__main__":

   game = Game()

