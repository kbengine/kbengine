/* test.c
An example of how to use nedalloc in C
(C) 2005-2010 Niall Douglas
*/

#define _CRT_SECURE_NO_WARNINGS 1	/* Don't care about MSVC warnings on POSIX functions */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "nedmalloc.h"

#define USE_NEDMALLOC_DLL

/**** TEST CONFIGURATION ****/
#if 0 /* Test patterns typical of C++ code */
#define THREADS 4					/* How many threads to run */
#define TESTCPLUSPLUS 1				/* =1 to make 50% of ops have blocksize<=512. This is typical for C++ allocator usage. */
#define BLOCKSIZE 16384				/* Test will be with blocks up to BLOCKSIZE. Try 8-16Kb for typical app usage, 1Mb if you use large arrays etc. */
#define TESTTYPE 2					/* =1 for maximum speed test, =2 for randomised test */
#define TOUCH 0						/* Whether to touch all pages of an allocated region. Can make a huge difference to scores. */
#define MAXMEMORY (768*1024*1024)	/* Maximum memory to use (approx) */
#define RECORDS (100000/THREADS)
#define MAXMEMORY2 (MAXMEMORY/THREADS)
#endif

#if 1 /* Test avrg. 2Mb block realloc() speed */
#define THREADS 1
#define TESTCPLUSPLUS 1
#define BLOCKSIZE (2*1024*1024)
#define TESTTYPE 2
#define TOUCH 1
#define MAXMEMORY (768*1024*1024)
#define RECORDS (400/THREADS)
#define MAXMEMORY2 (MAXMEMORY/THREADS)
#endif

#ifdef _MSC_VER
/*#pragma optimize("g", off)*/	/* Useful for debugging */
#endif

#if !defined(USE_NEDMALLOC_DLL)
#include "nedmalloc.c"
#elif defined(WIN32)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <malloc.h>
#else
#include <pthread.h>
#endif

#ifndef FORCEINLINE
  #if defined(__GNUC__)
#define FORCEINLINE __inline __attribute__ ((always_inline))
  #elif defined(_MSC_VER)
    #define FORCEINLINE __forceinline
  #endif
#endif
#ifndef NOINLINE
  #if defined(__GNUC__)
    #define NOINLINE __attribute__ ((noinline))
  #elif defined(_MSC_VER)
    #define NOINLINE __declspec(noinline)
  #else
    #define NOINLINE
  #endif
#endif


static int whichmalloc;
static int doRealloc;
static struct threadstuff_t
{
	struct
	{
		int mallocs;
		int reallocs;
		int frees;
	} ops;
	unsigned int *toalloc;
	void **allocs;
	char cachesync1[128];
	int done;
	char cachesync2[128];
} threadstuff[THREADS];

static void threadcode(int);

#ifdef WIN32
static DWORD WINAPI _threadcode(LPVOID a)
{
	threadcode((int)(size_t) a);
	return 0;
}
#define THREADVAR HANDLE
#define THREADINIT(v, id) (*v=CreateThread(NULL, 0, _threadcode, (LPVOID)(size_t) id, 0, NULL))
#define THREADSLEEP(v) SleepEx(v, FALSE)
#define THREADWAIT(v) (WaitForSingleObject(v, INFINITE), 0)

typedef unsigned __int64 usCount;
static FORCEINLINE usCount GetUsCount()
{
	static LARGE_INTEGER ticksPerSec;
	static double scalefactor;
	LARGE_INTEGER val;
	if(!scalefactor)
	{
		if(QueryPerformanceFrequency(&ticksPerSec))
			scalefactor=ticksPerSec.QuadPart/1000000000000.0;
		else
			scalefactor=1;
	}
	if(!QueryPerformanceCounter(&val))
		return (usCount) GetTickCount() * 1000000000;
	return (usCount) (val.QuadPart/scalefactor);
}

static HANDLE win32heap;
static void *win32malloc(size_t size)
{
	return HeapAlloc(win32heap, 0, size);
}
static void *win32realloc(void *p, size_t size)
{
	return HeapReAlloc(win32heap, 0, p, size);
}
static size_t win32memsize(void *p)
{
	return HeapSize(win32heap, 0, p);
}
static void win32free(void *mem)
{
	HeapFree(win32heap, 0, mem);
}

static void *(*const mallocs[])(size_t size)={ malloc, nedmalloc, win32malloc };
static void *(*const reallocs[])(void *p, size_t size)={ realloc, nedrealloc, win32realloc };
static size_t (*const memsizes[])(void *p)={ _msize, nedmemsize, win32memsize };
static void (*const frees[])(void *mem)={ free, nedfree, win32free };
#else
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#if defined(__cplusplus)
extern "C"
#else
extern
#endif
#if defined(__linux__) || defined(__FreeBSD__)
/* Sadly we can't include <malloc.h> as it causes a redefinition error */
size_t malloc_usable_size(void *);
#elif defined(__APPLE__)
size_t malloc_size(const void *ptr);
#else
#error Do not know what to do here
#endif
static void *_threadcode(void *a)
{
	threadcode((int)(size_t) a);
	return 0;
}
#define THREADVAR pthread_t
#define THREADINIT(v, id) pthread_create(v, NULL, _threadcode, (void *)(size_t) id)
#define THREADSLEEP(v) usleep(v*1000)
#define THREADWAIT(v) pthread_join(v, NULL)

typedef unsigned long long usCount;
static FORCEINLINE usCount GetUsCount()
{
#ifdef CLOCK_MONOTONIC
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((usCount) ts.tv_sec*1000000000000LL)+ts.tv_nsec*1000LL;
#else
	struct timeval tv;
	gettimeofday(&tv, 0);
	return ((usCount) tv.tv_sec*1000000000000LL)+tv.tv_usec*1000000LL;
#endif
}

static void *(*const mallocs[])(size_t size)={ malloc, nedmalloc };
static void *(*const reallocs[])(void *p, size_t size)={ realloc, nedrealloc };
static size_t (*const memsizes[])(void *p)={
#if defined(__linux__) || defined(__FreeBSD__)
malloc_usable_size,
#elif defined(__APPLE__)
malloc_size,
#endif
nedmemsize };
static void (*const frees[])(void *mem)={ free, nedfree };
#endif
static usCount times[THREADS];


static FORCEINLINE unsigned int myrandom(unsigned int *seed)
{
	*seed=1664525UL*(*seed)+1013904223UL;
	return *seed;
}

static void threadcode(int threadidx)
{
	int n;
	unsigned int *toallocptr=threadstuff[threadidx].toalloc;
	void **allocptr=threadstuff[threadidx].allocs;
	unsigned int r, seed=threadidx;
	usCount start;
	size_t allocated=0, size;
	threadstuff[threadidx].done=0;
	/*neddisablethreadcache(0);*/
	THREADSLEEP(100);
	start=GetUsCount();
#if 2==TESTTYPE
	/* A randomised malloc/realloc/free test (torture test) */
	for(n=0; n<RECORDS*100; n++)
	{
		static int reallocflip;
		unsigned int i, dorealloc=(reallocflip=!reallocflip);
		r=myrandom(&seed);
		i=(int)(r % RECORDS);
#if TESTCPLUSPLUS
		dorealloc=!(r&(15<<28));
		if(r&(1<<31))
		{   /* Make it two power multiple of less than 512 bytes to
			model frequent C++ new's */
			size=4<<(r & 7);
			dorealloc=0;
		}
		else
#endif
			size=(size_t)(r & (BLOCKSIZE-1));
		if(allocated<MAXMEMORY2 && !allocptr[i])
		{
			if(!(allocptr[i]=mallocs[whichmalloc](size))) abort();
#if TOUCH
			{
				volatile char *mem=(volatile char *)allocptr[i];
				volatile char *end=mem+size;
				for(; mem<end; mem+=4096) *mem;
			}
#endif
			allocated+=memsizes[whichmalloc](allocptr[i]);
			threadstuff[threadidx].ops.mallocs++;
		}
		else if(allocated<MAXMEMORY2 && dorealloc) /* If not TESTCPLUSPLUS, then how often realloc() gets called depends on how small RECORDS is. */
		{
			allocated-=memsizes[whichmalloc](allocptr[i]);
			if(!(allocptr[i]=reallocs[whichmalloc](allocptr[i], size))) abort();
#if TOUCH
			{
				volatile char *mem=(volatile char *)allocptr[i];
				volatile char *end=mem+size;
				for(; mem<end; mem+=4096) *mem;
			}
#endif
			allocated+=memsizes[whichmalloc](allocptr[i]);
			threadstuff[threadidx].ops.reallocs++;
		}
		else if(allocptr[i])
		{
			allocated-=memsizes[whichmalloc](allocptr[i]);
			frees[whichmalloc](allocptr[i]);
			allocptr[i]=0;
			threadstuff[threadidx].ops.frees++;
		}
	}
	for(n=0; n<RECORDS; n++)
	{
		if(allocptr[n])
		{
			allocated-=memsizes[whichmalloc](allocptr[n]);
			frees[whichmalloc](allocptr[n]);
			allocptr[n]=0;
			threadstuff[threadidx].ops.frees++;
		}
	}
	assert(!allocated);
#elif 1==TESTTYPE
	/* A simple stack which allocates and deallocates off the top (speed test) */
	for(n=0; n<RECORDS;)
	{
#if 1
		r=myrandom(&seed);
		if(allocptr>threadstuff[threadidx].allocs && (r & 65535)<32760) /*<32760)*/
		{	/* free */
			--toallocptr;
			--allocptr;
			--n;
			frees[whichmalloc](*allocptr);
			*allocptr=0;
			threadstuff[threadidx].ops.frees++;
		}
		else
#endif
		{
			if(doRealloc && allocptr>threadstuff[threadidx].allocs && (r & 1))
			{
	            if(!(allocptr[-1]=reallocs[whichmalloc](allocptr[-1], *toallocptr))) abort();
#if TOUCH
				{
					volatile char *mem=(volatile char *)allocptr[-1];
					volatile char *end=mem+*toallocptr;
					for(; mem<end; mem+=4096) *mem;
				}
#endif
				threadstuff[threadidx].ops.reallocs++;
			}
			else
			{
	            if(!(allocptr[0]=mallocs[whichmalloc](*toallocptr))) abort();
#if TOUCH
				{
					volatile char *mem=(volatile char *)allocptr[0];
					volatile char *end=mem+*toallocptr;
					for(; mem<end; mem+=4096) *mem;
				}
#endif
				threadstuff[threadidx].ops.mallocs++;
				allocptr++;
			}
			n++;
			toallocptr++;
			/*if(!(threadstuff[threadidx].ops & 0xff))
				nedtrimthreadcache(0,0);*/
		}
	}
	while(allocptr>threadstuff[threadidx].allocs)
	{
		frees[whichmalloc](*--allocptr);
		threadstuff[threadidx].ops.frees++;
	}
#endif
	times[threadidx]+=GetUsCount()-start;
	neddisablethreadcache(0);
	threadstuff[threadidx].done=1;
}

static double runtest()
{
	unsigned int seed=1;
	int n, i;
	double opspersec=0;
	THREADVAR threads[THREADS];
	for(n=0; n<THREADS; n++)
	{
		unsigned int *toallocptr;
		int m;
		memset(&threadstuff[n].ops, 0, sizeof(threadstuff[n].ops));
		times[n]=0;
		threadstuff[n].toalloc=toallocptr=calloc(RECORDS, sizeof(unsigned int));
		threadstuff[n].allocs=calloc(RECORDS, sizeof(void *));
		for(m=0; m<RECORDS; m++)
		{
			unsigned int size=myrandom(&seed);
#if TESTCPLUSPLUS
			if(size&(1<<31))
			{   /* Make it two power multiple of less than 512 bytes to
				model frequent C++ new's */
				size=4<<(size & 7);
			}
			else
#endif
			{
				size&=BLOCKSIZE-1;
			}
			*toallocptr++=size;
		}
	}
#if 2==TESTTYPE
	for(n=0; n<THREADS; n++)
	{
		THREADINIT(&threads[n], n);
	}
	for(i=0; i<8; i++)
	{
		int found=-1;
		do
		{
			for(n=0; n<THREADS; n++)
			{
				THREADSLEEP(100);
				if(threadstuff[n].done)
				{
					found=n;
					break;
				}
			}
		} while(found<0);
		THREADWAIT(threads[found]);
		threads[found]=0;
#if DEBUG
	  {
		  usCount totaltime=0;
		  int totalops=0, totalmallocs=0, totalreallocs=0;
		  for(n=0; n<THREADS; n++)
		  {
			  totaltime+=times[n];
			  totalmallocs+=threadstuff[n].ops.mallocs;
			  totalreallocs+=threadstuff[n].ops.reallocs;
			  totalops+=threadstuff[n].ops.mallocs+threadstuff[n].ops.reallocs;
		  }
		  opspersec=1000000000000.0*totalops/totaltime*THREADS;
		  printf("This test spent %f%% of its time doing reallocs\n", 100.0*totalreallocs/totalops);
		  printf("This allocator achieves %lfops/sec under %d threads\n\n", opspersec, THREADS);
	  }
#endif
		THREADINIT(&threads[found], found);
		printf("Relaunched thread %d\n", found);
	}
	for(n=THREADS-1; n>=0; n--)
	{
		THREADWAIT(threads[n]);
		threads[n]=0;
	}
#else
#if 1
	for(n=0; n<THREADS; n++)
	{
		THREADINIT(&threads[n], n);
	}
	for(n=THREADS-1; n>=0; n--)
	{
		THREADWAIT(threads[n]);
		threads[n]=0;
	}
#else
	/* Quick realloc() test */
	doRealloc=1;
	for(n=0; n<THREADS; n++)
	{
		THREADINIT(&threads[n], n);
	}
	for(n=THREADS-1; n>=0; n--)
	{
		THREADWAIT(threads[n]);
		threads[n]=0;
	}
#endif
#endif
	{
		usCount totaltime=0;
		int totalops=0, totalmallocs=0, totalreallocs=0;
		for(n=0; n<THREADS; n++)
		{
			totaltime+=times[n];
			totalmallocs+=threadstuff[n].ops.mallocs;
			totalreallocs+=threadstuff[n].ops.reallocs;
			totalops+=threadstuff[n].ops.mallocs+threadstuff[n].ops.reallocs;
		}
		opspersec=1000000000000.0*totalops/totaltime*THREADS;
		printf("This test spent %f%% of its time doing reallocs\n", 100.0*totalreallocs/totalops);
		printf("This allocator achieves %lfops/sec under %d threads\n", opspersec, THREADS);
	}
	for(n=THREADS-1; n>=0; n--)
	{
		free(threadstuff[n].allocs); threadstuff[n].allocs=0;
		free(threadstuff[n].toalloc); threadstuff[n].toalloc=0;
	}
	return opspersec;
}

int PatchInNedmallocDLL(void);
int main(void)
{
	double std=0, ned=0;
#if defined(WIN32) && defined(USE_NEDMALLOC_DLL)
	/*PatchInNedmallocDLL();*/
#endif

#if 0
	{
		usCount start, end;
		start=GetUsCount();
		THREADSLEEP(5000);
		end=GetUsCount();
		printf("Wait was %lf\n", (end-start)/1000000000000.0);
	}
#endif
#ifdef WIN32
#pragma comment(lib, "user32.lib")
	{	/* Force load of user32.dll so we can debug */
		BOOL v;
		SystemParametersInfo(SPI_GETBEEP, 0, &v, 0);
	}
#endif
#if 2==TESTTYPE
	printf("Running torture test\n"
		   "-=-=-=-=-=-=-=-=-=-=\n");
#elif 1==TESTTYPE
	printf("Running speed test\n"
		   "-=-=-=-=-=-=-=-=-=\n");
#endif
	printf("Block size <= %u, C++ test mode is %s\n", BLOCKSIZE, TESTCPLUSPLUS ? "on" : "off");
	if(0)
	{
		printf("\nTesting standard allocator with %d threads ...\n", THREADS);
		std=runtest();
	}
	if(1)
	{
		printf("\nTesting nedmalloc with %d threads ...\n", THREADS);
		whichmalloc=1;
		ned=runtest();
	}
#ifdef WIN32
	if(0)
	{
		ULONG data=2;
		win32heap=HeapCreate(0, 0, 0);
		HeapSetInformation(win32heap, HeapCompatibilityInformation, &data, sizeof(data));
		HeapQueryInformation(win32heap, HeapCompatibilityInformation, &data, sizeof(data), NULL);
		if(2!=data)
		{
			printf("The win32 low frag allocator won't work under a debugger!\n");
		}
		else
		{
			printf("Testing win32 low frag allocator with %d threads ...\n\n", THREADS);
			whichmalloc=2;
			runtest();
		}
		HeapDestroy(win32heap);
	}
#endif
	if(std && ned)
	{	// ned should have more ops/sec
		printf("\n\nnedmalloc allocator is %lf times faster than standard\n", ned/std);
	}
	printf("\nPress a key to trim\n");
	getchar();
	nedmalloc_trim(0);
#ifdef _MSC_VER
	printf("\nPress a key to end\n");
	getchar();
#endif
	return 0;
}
