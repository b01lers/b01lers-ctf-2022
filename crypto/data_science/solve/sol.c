#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double A, B, C;
const int N = 1000;
const double sgoal = -M_PI;
const double NONE = 1e100;

//-- do the sum, exactly as in the challenge

double doSum(double a, double b) {
   // fill histogram
   double x[N + 1];
   double y[N + 1];
   for (int i = 0; i <= N; i ++) {
      x[i] = a + i * (b - a) / N;
      y[i] = A * x[i] * x[i] + B * x[i] + C;
   }

   // transform
   double xlo = (-B + sqrt(B * B - 4. * A * C)) / (2. * A);
   double xhi = (-B - sqrt(B * B - 4. * A * C)) / (2. * A);
   double xmid = (xlo + xhi) * 0.5;
   for (int i = 0; i <= N; i ++) {
      if (x[i] >= xlo && x[i] <= xmid) y[i] = sqrt(y[i]);
   }

   // find kmax, k0
   int kmax = -1, imax = -1;
   double ymax = 0.;
   for (int i = 0; i <= N; i ++) {
      if (y[i] > ymax) {
         ymax = y[i];
         kmax = (ymax - a) * N / (b - a);
         imax = i;
      }
   }
   int k0 = kmax;
   for (int i = imax; i >= 0; i --) {
      if (y[i] > 0.) continue;
      k0 = (y[i] - a) * N / (b - a);
      break;
   }

   // histogram & sum in one
   double s = 0.;
   int kprev = kmax;
   for (int i = 0; i <= N; i ++) {
      if (x[i] > xmid) continue;
      int k = (y[i] - a) * N / (b - a);
      if (k < k0) continue;
      if (k > kmax) continue;
      if (k == kprev) continue;   // avoid double-counting bin
      kprev = k;
      s += x[i] - xlo;
   }
   s *= (b - a) / N;

   return s;
}


// solve xlo = a + i * (b - a) / N  for b
//  => b =  (xlo - a) * N / i + a
// indicate when there is no valid solution (rounding errors, or b <= a)
//
double bSolve(double xlo, double a, int i) {
   double b0 = (xlo - a) * N / i + a;
   double xlo0 = a + i * (b0 - a) / N;
   if (xlo0 == xlo) return (b0 > a) ? b0 : NONE;
   // try to tune, if need be
   // this could likely be sped up/replaced by more careful rounding analysis
   // but binary root search is simple and solid
   double b1 = b0;
   double eps = 1e-8;
   if (xlo0 < xlo) {
      while (xlo0 < xlo) {
         b1 += eps;
         xlo0 = a + i * (b1 - a) / N;
         //printf("xlo0=%.17g xlo=%.17g b1=%.17g a=%.17g\n", xlo0, xlo, b1, a);
      }
   }
   else {
      while (xlo0 > xlo) {
         b1 -= eps;
         xlo0 = a + i * (b1 - a) / N;
         //printf("xlo0=%.17g xlo=%.17g b1=%.17g\n", xlo0, xlo, b1);
      }
   }
   while (1) {
      double b2 = (b0 + b1) * 0.5;
      if (b2 == b0 || b2 == b1) break;
      double xlo2 = a + i * (b2 - a) / N;
      if (xlo2 == xlo) return (b2 > a) ? b2 : NONE;
      else if (xlo2 < xlo) b0 = b2;
      else b1 = b2;
   }
   return NONE;  // no precise solution
}


// tune 'a' until [a,b] with a valid 'b' from bSolve() for the given 'i'
// yields an integral that is smaller than -pi
//
// stop when 'a' is too large (rounding errors start to spoil getting a valid 'b')
// simple heuristic alg, may fail - exploits approximate monotonicity as 'a' decreases
//
double aTune(double xlo, double a0, int i) {
   double b = bSolve(xlo, a0, i);
   if (b == NONE) return NONE;
   double s0 = doSum(a0, b);
   if (s0 == sgoal) return a0;
   // tune
   double delta = 0.02 * fabs(xlo);
   //double MAX = 10. * fabs(xlo);
   double MAX = 100. * fabs(xlo);
   double a1 = a0, s1 = s0;
   double abest = a0, sbest = NONE;
   if (s0 < sgoal) {
      while(s1 < sgoal) {
         a1 += delta;
         if (fabs(a1) > MAX) return abest;
         double b1 = bSolve(xlo, a1, i);
         if (b1 == NONE) continue;
         s1 = doSum(a1, b1);
         if (s1 < sbest) {  sbest = s1;   abest = a1;  }
         //printf("a1=%.17g b1=%.17g s1=%.17g\n", a1, b1, s1);
      }
   }
   else {
      while(s1 > sgoal) {
         a1 -= delta;
         if (fabs(a1) > MAX) return abest;
         double b1 = bSolve(xlo, a1, i);
         if (b1 == NONE) continue;
         s1 = doSum(a1, b1);
         if (s1 < sbest) {  sbest = s1;   abest = a1;  }
         //printf("a1=%.17g b1=%.17g s1=%.17g\n", a1, b1, s1);
      }
   }
   printf("a0=%.17g a1=%.17g s0=%.17g s1=%.17g\n", a0, a1, s0, s1);
   return a1;
}


// search for exact solution via tuning [a,b]
void solve(double xlo, double* ares, double* bres) {   
   // get a starting point
   for (int i = N; i >= 0; i --) {
      double a0 = aTune(xlo, xlo - 1., i);
      if (a0 == NONE) continue;
      double b0 = bSolve(xlo, a0, i);
      if (b0 == NONE) continue;
      double s0 = doSum(a0, b0);
      if (s0 > sgoal) continue;
      else if (s0 == sgoal) {
         *ares = a0;
         *bres = b0;
         break;
      }
      double a1 = a0 + 0.02 * fabs(xlo); // undo last decrement in aTune()
      double b1 = bSolve(xlo, a1, i);
      if (b1 == NONE) continue;
      double s1 = doSum(a1, b1);
      if (s1 == sgoal) {
         *ares = a0;
         *bres = b0;
         return;
      }
      if ((s1 - sgoal) * (s0 - sgoal) > 0.) continue;
      // iterate within [a0, a1]
      double a2, b2, s2;
      while (1) {
         a2 = (a0 + a1) * 0.5;
         if (a2 == a0 || a2 == a1) {
            *ares = a2;
            *bres = b2;
            return;
         }
         b2 = bSolve(xlo, a2, i);
         if (b2 == NONE) break;
         double s2 = doSum(a2, b2);
         if (s2 == sgoal) {
            *ares = a2;
            *bres = b2;
            return;
         }
         else if (s2 < sgoal) a0 = a2;
         else a1 = a2;
     }
   }
   // indicate failure (set a=b)
   *ares = 0.;
   *bres = 0.;
}


int main(int ARGC, const char** const ARGV) {

   // read coeffs
   A = (ARGC > 1) ? atof(ARGV[1]) : -1.;
   B = (ARGC > 2) ? atof(ARGV[2]) : 2.;
   C = (ARGC > 3) ? atof(ARGV[3]) : 1.;

   printf("A=%.17g B=%.17g C=%.17g\n", A, B, C);

   // derived points
   double xlo = (-B + sqrt(B * B - 4. * A * C)) / (2. * A);
   double xhi = (-B - sqrt(B * B - 4. * A * C)) / (2. * A);
   double xmid = (xlo + xhi) * 0.5;

   if (isnan(xlo)) {
      printf("invalid A,B,C\n");
      return 1;
   }

   printf("xlo=%.17g\n", xlo);
   double flo = A * xlo * xlo + B * xlo + C;
   if (flo >= 0.) {
      printf("unsuitable A,B,C\n");
      return 1;
   }

   // solve
   double a, b;
   solve(xlo, &a, &b);
   double s = (a == b) ? 0. : doSum(a, b);
   printf("sol: a=%.17g b=%.17g s=%.17g sgoal=%.17g\n", a, b, s, sgoal);
   if (s == sgoal) printf("BINGO!\n");

   // OK
   return 0;
}
