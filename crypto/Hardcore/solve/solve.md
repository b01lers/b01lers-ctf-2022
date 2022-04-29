# Hardcore Challenge Write up

This challenge is based off the proof for the Goldreich-Levin Theorem.  A great overview can be found here [Cornell Slides](http://www.cs.cornell.edu/courses/cs687/2006fa/lectures/lecture11.pdf).

## Premise

A hardcore predicate is a generated a bit that *should* be impossible to predict. More formally if you have a secret $x$ and some randomness  $r$ of the same length $n$, you can generate a hardcore bit $h(x, r) = \sum_i^n x*r \  (mod\ 2)$ (ie. the dot product of $x$ and $r$) and publish both $f(x), r$ where $f$ is some one way function. It was shown by Goldreich and Levin that with this protocol, no Turing Machine $A(f(x), r)$ taking $f(x), r$ as arguments can predict the hardcore bit with probability $>\frac{1}{2} + neg(n)$ which is basically only as good as guessing (for more information on $neg(n)$ check out the [Wikipedia](https://en.wikipedia.org/wiki/Negligible_function), but it just means a really really small function).

## Main Idea

In order to prove this, Goldreich and Levin showed by contradiction that if a machine $A$ could predict the hardcore bit, that this would allow you to undo the one way function $f$ by recovering $x$. They did this in 3 stages: predictor with 100% accuracy, predictor with 90% accuracy, and finally a predictor with $>\frac{1}{2} + neg(n)$

This is exactly what our challenge asks you to do, to take a predictor function we give and use it to recover the secret/flag $x$ encoded with SHA256. 

To see attack-script pseudocode and proof of correctness see the [Cornell Slides](http://www.cs.cornell.edu/courses/cs687/2006fa/lectures/lecture11.pdf) otherwise you can see an exact attack script in `Solve1.py` and `Solve2.py`.
