#include "common.h"

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
