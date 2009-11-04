#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "try.h"
using namespace std;

void test(){}

int main()
{
   FILE* file = fopen("blah.bin", "wb+");
   int array[4];
   array[2] = 100;
   for(int k = 0; k < 4; k++)
      cout << array[k] << endl;
   fwrite(array, sizeof(int), 4, file);
   fclose(file);
   file = fopen("blah.bin", "wb+");
   fread(array, sizeof(int), 4, file);
   for(int k = 0; k < 4; k++)
      cout << array[k] << endl;
   fpos_t position;
   fgetpos(file, &position);
   int number[] = {1, 2, 3, 4, 5};
   int change[] = {10, 11};
   fwrite(number, sizeof(int), 5, file);
   fclose(file);
   file = fopen("blah.bin", "r+b");
   fseek(file, 0, SEEK_SET);
   fwrite(change, sizeof(int), 2, file);
   fclose(file);
   file = fopen("blah.bin", r);
   int* result = (int*) malloc(5*sizeof(int));
   fread(result, sizeof(int), 5, file);
   for(int k=0; k<5; k++)
      cout<<*(result+k)<<endl;
   fclose(file);
   return 0;
}
