/* make_pgos.c
Simply runs through each of the most common code paths for PGO purposes
(C) 2010 Niall Douglas
*/

#define _CRT_SECURE_NO_WARNINGS 1	/* Don't care about MSVC warnings on POSIX functions */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "nedmalloc.h"

#define RECORDS 100000

static void run(size_t maxsize, size_t records, size_t loops)
{
	int n,m;
	static void *mem[RECORDS];
	for(m=0; m<loops; m++)
	{
		for(n=0; n<records-1; n++)
		{
			static int reallocflip;
			int dorealloc=(reallocflip=!reallocflip);
			size_t size=rand() & (maxsize-1);
			if(!mem[n])
				mem[n]=nedmalloc(size);
			else if(n>0 && dorealloc)
				mem[n-1]=nedrealloc(mem[n-1], size);
			else if(n>1)
			{
				nedfree(mem[n-2]);
				mem[n-2]=0;
			}
		}
	}
	for(n=0; n<records; n++)
	{
		if(mem[n])
		{
			nedfree(mem[n]);
			mem[n]=0;
		}
	}
}

int main(void)
{
	printf("nedmalloc PGO maker\n"
		   "-=-=-=-=-=-=-=-=-=-\n");
	printf("Small allocations\n");
	run(8192, RECORDS, 10);

	printf("Medium allocations\n");
	run(256*1024, RECORDS/100, 1000);

	printf("Large allocations\n");
	run(8*1024*1024, RECORDS/1000, 10000);

	return 0;
}