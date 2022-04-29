SHArdle - solution
==================

This is just like the Wordle game, except that the hash of the generated word is being
compared to the hash of the guess. Obviously, with enough guesses you can zero in on the
hash but you would still need to invert that for the secret word. This is SHA256, i.e.,
good luck. So step 1 is to realize that, for this to be solvable, the game must have
some manageable set of secrets for which you can precompute hashes. 

Step 2 is figuring out what is accepted. valid() and generate() are in an unknown secret
file, so you must experiment. With lower() on the input, only lowercase matters. After
some guesses, you can see that English words are accepted, while words with numbers,
punctuation, or phrases are not (so this is not a list of weak passwords, for example).
You can then either try to narrow down the dictionary - e.g., by checking unusual words 
(the challenge actually uses a scrabble dictionary, about 700k words); or grab the
largest online word list you can. I would pick the latter - there is a pretty big ~1.5M 
word list at https://www.keithv.com/software/wlist/ .

Step 3, guessing strategy. You have 15 attemps in total at 2 rounds - *pretty* generous,
actually. A SHA256 hash consists of 64 hex digits. Assuming that your guess hash is random, on
average you get 4 digits green with each guess; so very crudely you cut the search space
by 16^4 = 64k on each guess. Of course, this fluctuates - sometimes there are only 2-3 greens,
and greens from different guesses might overlap. Still, with just 3 fixed guesses, you
can pinpoint the secret with practically 100% probability, as long as the word is in your
dictionary. So you do not need to analyze any yellows/blacks at all, and you win both rounds
with just 6 guesses in total.

The rest is straightforward coding.

(Btw, in case you were wondering about generate(randint)... randint() here is a cryptographically
secure RNG being passed to generate(), so this was a hint that for practical purposes generation
is truly random and the two secrets are independent.)
