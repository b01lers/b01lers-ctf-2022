Data Science
============

This challenge

  - creates an (x,y) table for an inverted parabola in the range a <= x <= b, equidistant x, 1000 + 1 points
  - takes square root of the positive (y > 0) points
  - then effectively rotates the plot 90 degrees CCW, and tabulates the result using the same grid as in step 1
  - finally it shifts the curve along the new vertical so that it goes through the origin and uses the grid
    to compute the integral under the section of the plot that is positive, up to at most tip of the parabola.
    (Integration only happens in spirit because it is technically wrong - the code does avoid double-counting
     when neighboring points fall into the same histogram bin, and works well when neighboring points fall into
     neighboring bins. However, the math does not handle the case when there is a gap between neighboring points.)

Tabulation, shifting and integration are done in one step so no extra storage is used. The challenge is that the
integral is seemingly positive (nonnegative), but you can only win if your end result is minus pi.

Two things can be exploited:

  1) the positivity check in the final integration is off by one, so one negative histogram bin may contribute.
     However, the largest bin size is a little shy of 0.8 (b - a < 800), so this is insufficient for reaching a
     negative enough result. The baby challenge is designed to be solvable this way - barely.

  2) the y>0 condition for the sqrt is done via comparison to the roots of the parabola. Depending on the coeffs 
     A, B, C (about 50% chance), the parabola evaluates to negative at the root due to round-off error in 64-bit
     FP, so sqrt yields -nan. Upon histogramming, this gives a large negative bin index, fooling the range checks
     in the final integration. Done correctly, this gives you many bins with negative function values.

     You then need to carefully tune the [a,b] range to achieve precisely minus pi within 64-bit FP precision.
     Round-off errors can spoil this, ofc, but there is a reasonable O(1-10%) overall chance of success even with
     my relatively crappy alg.
