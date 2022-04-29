import gzip

FLAG = open("./flag.txt", "r").read()

## WORDS
def readDict(fname):
   with gzip.open(fname, "rb") as f:
      data = f.read()
   return data.split()

wordList = readDict("./sowpods.txt.gz")

def generate(f): return wordList[f(0, len(wordList) - 1)] 

def valid(s):  return s in wordList
