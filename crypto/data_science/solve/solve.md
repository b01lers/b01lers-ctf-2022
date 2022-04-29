Data Science - solver
=====================

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

     You then need to carefully tune the [a,b] range to achieve precisely minus pi within 64-bit FP precision.
     Round-off errors can spoil this, ofc, but there is a good overall chance of success even with my crude alg.

----

To get a nan at the root, you obviously need to pick the grid such that one grid point falls onto the root. In the loop that
determines kmax and k0, the >0 check will fail for the nan, so k0 = int(-nan) = -2147483648 (on x86). In which case, practically
all points of the curve with k <= kmax will contribute to the final sum.

The solver is composed of four ingredients/subtasks:

  i) precisely invert   xlo = a + i * (b - a) / N   for b, or catch when that is impossible

  This ensures that given a & i, the i-th gridpoint in [a,b] will fall on xlo. Standard linear algebra + back substitution
  as crosscheck, and if that fails, bisection rootsearch to be absolutely sure.


  ii) find lower and upper bounds a1 & a2 for 'a' such that with the correspoding 'b'-s from [step i)] the integrals bracket -M_PI.

  We start with a = xlo - 1 and gradually lower 'a'. For each 'a', we determine 'b' using step i), compute the integral, and keep
  iterating until we undershoot -M_PI. This then gives us a1,  we then take the previous 'a' as a2.

  
  iii) tune 'a' between a1 and a2 until the corresponding [a,b] interval gives -M_PI for the integral.
  
  The expectation is that the integral varies smoothly with 'a', so this can be achieved with the standard bisecting rootfinder.


  iv) try the above for each i from N to 0, until we succeed... or fail. 

  If A, B, C are unsuitable, or we fail for all 'i', reconnect for a new challenge.

==END==
