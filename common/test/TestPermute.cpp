#include <iostream>
#include "../common.h"
#include <time.h>

int main()
{
	srand(time(NULL));
	int a[10];
	kPermute(a, 5, 20, 10);
	for (int i=0; i<10; i++)
		printf("%d ", a[i]);
	printf("\n");
}
