
#include <assert.h>
#include <iostream>
#include <string.h>
#include "MDCoord.h"
using namespace std;

MDCoord::MDCoord(u8 nDims, ...)
{
   this->nDims = nDims;
   if (nDims == 0)
      this->coords = NULL;
   else
   {
      this->coords = new i64[nDims];
      va_list args;
      va_start(args, nDims);
      for (int k = 0; k < nDims; k++)
         this->coords[k] = va_arg(args, i64);
      va_end(args);
   }
}

MDCoord::MDCoord(i64 *coords, u8 nDims)
{
   this->nDims = nDims;
   if (nDims == 0)
      this->coords = NULL;
   else
   {
      this->coords = new i64[nDims];
      memcpy(this->coords, coords, nDims*sizeof(i64));
//      for (int k = 0; k < nDims; k++)
  //       this->coords[k] = coords[k];
   }
}

MDCoord::~MDCoord()
{
    if (coords != NULL)
        delete[] coords;
}

MDCoord::MDCoord(const MDCoord &src)
{
   nDims = src.nDims;
   if (nDims == 0)
      coords = NULL;
   else
   {
      coords = new i64[nDims];
      for (int k = 0; k < nDims; k++)
         coords[k] = src.coords[k];
   }
}

MDCoord & MDCoord::operator=(const MDCoord &src)
{
   if (this != &src)
   {
      if (src.nDims == 0)
      {
         nDims = 0;
         if (coords != NULL)
             delete[] coords;
         coords = NULL;
      }
      else 
      {
         if (nDims != src.nDims)
         {
            delete[] coords;
            nDims = src.nDims;
            coords = new i64[nDims];
         }
         for (int k = 0; k < nDims; k++)
            coords[k] = src.coords[k];
      }
   }

   return *this;
}

bool MDCoord::operator==(const MDCoord &other) const
{
   if (nDims != other.nDims)
      return false;

   for (int k = 0; k < nDims; k++)
   {
      if (coords[k] != other.coords[k])
         return false;
   }
   return true;
}

MDCoord & MDCoord::operator+=(const MDCoord &other)
{
   assert(nDims == other.nDims);

   for (int k = 0; k < nDims; k++)
      coords[k] += other.coords[k];

   return *this;
}

MDCoord & MDCoord::operator-=(const MDCoord &other)
{
   assert(nDims == other.nDims);

   for (int k = 0; k < nDims; k++)
      coords[k] -= other.coords[k];

   return *this;
}

