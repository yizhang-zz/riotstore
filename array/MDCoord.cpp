
#include <assert.h>
#include <iostream>
#include <string.h>
#include "MDCoord.h"
using namespace std;

MDCoord::MDCoord(u8 nDim, ...)
{
   this->nDim = nDim;
   if (nDim == 0)
      this->coords = NULL;
   else
   {
      this->coords = new i64[nDim];
      va_list args;
      va_start(args, nDim);
      for (int k = 0; k < nDim; k++)
         this->coords[k] = va_arg(args, i64);
      va_end(args);
   }
}

MDCoord::MDCoord(i64 *coords, u8 nDim)
{
   this->nDim = nDim;
   if (nDim == 0)
      this->coords = NULL;
   else
   {
      this->coords = new i64[nDim];
      memcpy(this->coords, coords, nDim*sizeof(i64));
   }
}

MDCoord::~MDCoord()
{
    if (coords != NULL)
        delete[] coords;
}

MDCoord::MDCoord(const MDCoord &src)
{
   this->nDim = src.nDim;
   if (this->nDim == 0)
      coords = NULL;
   else
   {
      this->coords = new i64[nDim];
      memcpy(this->coords, src.coords, nDim*sizeof(i64));
   }
}

MDCoord & MDCoord::operator=(const MDCoord &src)
{
   if (this != &src)
   {
      if (src.nDim == 0)
      {
         nDim = 0;
         if (coords != NULL)
             delete[] coords;
         coords = NULL;
      }
      else 
      {
         if (nDim != src.nDim)
         {
            delete[] coords;
            nDim = src.nDim;
            coords = new i64[nDim];
         }
         memcpy(coords, src.coords, nDim*sizeof(i64));
         /*for (int k = 0; k < nDim; k++)
            coords[k] = src.coords[k];*/
      }
   }

   return *this;
}

bool MDCoord::operator==(const MDCoord &other) const
{
   if (nDim != other.nDim)
      return false;

   for (int k = 0; k < nDim; k++)
   {
      if (coords[k] != other.coords[k])
         return false;
   }
   return true;
}

MDCoord & MDCoord::operator+=(const MDCoord &other)
{
   assert(nDim == other.nDim);

   for (int k = 0; k < nDim; k++)
      coords[k] += other.coords[k];

   return *this;
}

MDCoord & MDCoord::operator-=(const MDCoord &other)
{
   assert(nDim == other.nDim);

   for (int k = 0; k < nDim; k++)
      coords[k] -= other.coords[k];

   return *this;
}

string MDCoord::toString() const
{
    char buf[256] = "[";
    char num[20];
    if (nDim > 0) {
        sprintf(num, "%lld", coords[0]);
        strcat(buf, num);
    }
    for (int i=1; i<nDim; i++) {
        sprintf(num, " %lld", coords[i]); 
        strcat(buf, num);
    }
    strcat(buf, "]");
    return string(buf);
}
