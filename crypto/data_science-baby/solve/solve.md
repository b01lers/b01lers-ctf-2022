Baby Data Science - solver
==========================

The challenge

  - creates an (x,y) table for an inverted parabola in the range a <= x <= b, equidistant x, 1000 + 1 points

  - takes square root of the positive (y > 0) points

  - then effectively rotates the plot 90 degrees CCW, and tabulates the result using the same grid as in step 1

  - finally it shifts the curve along the new vertical so that it goes through the origin, and uses the grid
    to compute the integral under the section of the plot that is positive, up to at most tip of the parabola.

    (Integration only happens in spirit because it is technically wrong - the code does avoid double-counting
     when neighboring points fall into the same histogram bin, and works well when neighboring points fall into
     neighboring bins. However, the math does not cover the case when there is a gap between neighboring points.)

Tabulation, shifting and integration are all done in one, so no extra storage is used. The challenge is that the
integral is seemingly positive (nonnegative), but you can only win if your end result is minus pi.


Two things can be exploited:

  1) the positivity check in the final integration is off by one, so one negative histogram bin may contribute.
     However, the largest bin size is a little shy of 0.8 (b - a < 800), so this is insufficient for reaching a
     negative enough result. The baby challenge is designed to be solvable this way - barely.

  2) the y>0 condition for the sqrt is done via comparison to the roots of the parabola. Depending on the coeffs 
     A, B, C (about 50% chance), the parabola evaluates to negative at the root due to round-off error in 64-bit
     FP, so sqrt yields -nan. Upon histogramming, this gives a large negative bin index, fooling the range checks
     in the final integration. Done correctly, this gives you many bins with negative function values.

     For more details on exploiting the nan, check the solver for Data Science.

----

Some useful observations for the baby challenge (ignoring the nan trick):

  - your score (integral) needs to be -M_PI + 2.51 = -0.63159... or lower
  - only the last summed bin can be negative, the earlier ones are positive
  - at least one positive bin gets counted
  - bin spacing is (b - a) / N in x
  - |a| and |b| are capped at 400

=> so the goal is to make one positive bin practially zero, while the negative one as negative as possible

For that, you need to place the positive bin at xlo + epsilon, and make (b - a) as large as possible.

The first idea might be to set b = xlo + epsilon, and 'a' close to -400. However, that only gives bin spacing 
(b - a) / N = 0.4 or so, so the integral will be roughly -(0.4)^2 = -0.16. While the limit is about -0.63. 

For this to work, you need almost double the range, i.e., use as much of the full (-400,400) as possible,
while still keeping a grid point at xlo. Then, spacing is about 0.8, and you could reach almost -0.64.
But that brings in additional positive bins... well, it usually does. However, with the A, B, C coefficients
given, there is a finite O(1%) chance that the parabola is so narrow that 

    xmid - xlo = sqrt(B^2 - 4 A C) / (-2A)    < 0.8

    (the minimum of this is at B = 2, A = 2, C = 0.4 => roughly 0.67)

and then there is only a single positive bin (the one you placed near xlo). You just keep trying 
until A,B,C are suitable, and then it works.

==END==
