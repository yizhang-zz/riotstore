#include "common.h"
#include <math.h>

static double R_ValueOfNA()
{
   /* The gcc shipping with RedHat 9 gets this wrong without
    * the volatile declaration. Thanks to Marc Schwartz. */
   volatile ieee_double x;
   x.word[hw] = 0x7ff00000;
   x.word[lw] = 1954;
   return x.value;
}

double NA_DOUBLE = R_ValueOfNA();
// int NA_INT = INT32_MIN;

bool R_IsNA(double x)
{
   if (isnan(x)) 
   {
      ieee_double y;
	  y.value = x;
	  return (y.word[lw] == 1954);
   }
   return false;
}
