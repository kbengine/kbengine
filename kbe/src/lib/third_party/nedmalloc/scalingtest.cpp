/* scalingtest.cpp
Tests how various allocators scale according to block size using a monte-carlo approach
(C) 2010 Niall Douglas
*/

#define FORCEINLINE
#define NOINLINE
#define ENABLE_USERMODEPAGEALLOCATOR 1

//#define THREADCACHEMAX 0
//#pragma optimize("g", off)

#include "nedmalloc.c"
#include <math.h>
#include <vector>

#define LOOPS 25000
#define MAXBLOCKSIZE (8*1024*1024)

#ifdef WIN32
typedef unsigned __int64 usCount;
static usCount GetUsCount()
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
#else
#include <sys/time.h>

typedef unsigned long long usCount;
static usCount GetUsCount()
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
#endif

template<void (*_free)(void *)> void wrapfree(void *mem, size_t size)
{
	_free(mem);
}
static void *mmap_wrapper(size_t size)
{
#ifdef WIN32
	return VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#else
	return mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#endif
}
static void *mmappop_wrapper(size_t size)
{
#ifdef WIN32
#else
	return mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_POPULATE, -1, 0);
#endif
}
static void munmap_wrapper(void *mem, size_t size)
{
#ifdef WIN32
	VirtualFree(mem, 0, MEM_RELEASE);
#else
	munmap(mem, size);
#endif
}
static void *userpagemalloc_wrapper(size_t size)
{
	assert(!(size & (PAGE_SIZE-1)));
	return userpage_malloc(size, 0);
}
static void userpagefree_wrapper(void *mem, size_t size)
{
	assert(!(size & (PAGE_SIZE-1)));
	userpage_free(mem, size);
}
static mspace mymspace = create_mspace(0,0);
static void *dlmalloc(size_t size)
{
	return mspace_malloc(mymspace, size);
}
static void dlfree(void *mem, size_t size)
{
	mspace_free(mymspace, mem);
}
static void *nothing_malloc(size_t size)
{
	return 0;
}
static void nothing_free(void *mem, size_t size)
{
}

struct Allocator
{
	const char *name, *shortname;
	size_t minsize, minsizeshift;
	bool traverse;
	void *(*malloc)(size_t);
	void (*free)(void *, size_t);
};
static Allocator allocators[]={
	{ "System allocator", "sysalloc", 0, 0, true, &malloc, &wrapfree<free> },
	{ "nedmalloc", "nedmalloc", 0, 0, true, &nedalloc::nedmalloc, &wrapfree<nedalloc::nedfree> },
	{ "dlmalloc", "dlmalloc", 0, 0, true, &dlmalloc, &dlfree },
#ifndef WIN32
	{ "System mmap()", "sysmmap", PAGE_SIZE, 0, true, &mmap_wrapper, &munmap_wrapper },
	{ "System mmap(MAP_POPULATE)", "sysmmappop", PAGE_SIZE, 0, true, &mmappop_wrapper, &munmap_wrapper },
	{ "System mmap(MAP_POPULATE, notraverse)", "sysmmappop_notraverse", PAGE_SIZE, 0, false, &mmappop_wrapper, &munmap_wrapper },
#else
	{ "System VirtualAlloc()", "sysmmap", PAGE_SIZE, 0, true, &mmap_wrapper, &munmap_wrapper },
#endif
	{ "User mode page allocator", "usermodemmap", PAGE_SIZE, 0, true, &userpagemalloc_wrapper, &userpagefree_wrapper },
	{ "User mode page allocator (notraverse)", "usermodemmap_notraverse", PAGE_SIZE, 0, true, &userpagemalloc_wrapper, &userpagefree_wrapper },
	{ "Nothing", "nothing", 0, 0, false, &nothing_malloc, &nothing_free }
};

int main(void)
{
	using namespace std;
	printf("What would you like to test?\n");
	for(unsigned n=0; n<sizeof(allocators)/sizeof(Allocator); n++)
	{
		printf("   %u. %s\n", n+1, allocators[n].name);
	}
	unsigned allocatoridx=getchar()-'1';
	if(allocatoridx<0 || allocatoridx>sizeof(allocators)/sizeof(Allocator)) return 1;
	Allocator &allocator=allocators[allocatoridx];
	allocator.minsizeshift=allocator.minsize ? nedtriebitscanr(allocator.minsize) : (allocator.minsize=1<<3, 3);
	printf("\nYou chose allocator %u (%s) with minsizeshift=%lu\n", allocatoridx+1, allocator.name, (unsigned long) allocator.minsizeshift);
	//if(allocator.malloc==&userpagemalloc_wrapper)
	{
		//printf("Preallocating user mode page allocator memory ... \n");
		//userpage_free(userpage_malloc(3ULL*1024*1024*1024, 0), 3ULL*1024*1024*1024);
	}
	for(usCount s=GetUsCount(); GetUsCount()-s<3000000000000ULL;);
	printf("Testing ...\n");

	vector<pair<usCount, size_t> > bins(nedtriebitscanr(MAXBLOCKSIZE)+1);
	struct Ptrs_t { void *mem; size_t size; } ptrs[512], *ptrp;
	memset(ptrs, 0, sizeof(ptrs));
	ptrp=ptrs;
	for(int n=0; n<LOOPS; n++)
	{
		size_t blksize;
		do
		{
			double randval=(double) rand()/(RAND_MAX/(8*sizeof(size_t)))+allocator.minsizeshift;
			blksize=((size_t) pow(2, randval)) & (MAXBLOCKSIZE-1);
			blksize&=~(allocator.minsize-1);
		} while(blksize<allocator.minsize);
		usCount start, end;
		if(n>LOOPS/2 && blksize<32)
		{
			int a=1;
		}
		if(ptrp->mem)
		{
			start=GetUsCount();
			allocator.free(ptrp->mem, ptrp->size);
			end=GetUsCount();
			pair<usCount, size_t> &v=bins[nedtriebitscanr(ptrp->size)];
			v.first+=end-start;
			ptrp->mem=0; ptrp->size=0;
		}
		start=GetUsCount();
		ptrp->mem=allocator.malloc(blksize);
		ptrp->size=blksize;
		//if(allocator.malloc!=&userpagemalloc_wrapper)
		{
			if(ptrp->mem && allocator.traverse)
			{
				for(volatile char *p=(volatile char *)ptrp->mem, *pend=(volatile char *)ptrp->mem+blksize; p<pend; p+=PAGE_SIZE)
					*p=1;
			}
		}
		end=GetUsCount();
		if(++ptrp==ptrs+512) ptrp=ptrs;
		pair<usCount, size_t> &v=bins[blksize ? nedtriebitscanr(blksize) : 0];
		v.first+=end-start;
		v.second++;
	}
	char filename[256];
	sprintf(filename, "scalingtest%s_%s.csv",
#ifdef WIN32
		"_win32",
#else
		"_posix",
#endif
		allocator.shortname);
	printf("\nWriting results to %s ...\n", filename);
	FILE *oh=fopen(filename, "w");
	fprintf(oh, "Bin,Latency,Count\n");
	int n=0;
	for(vector<pair<usCount, size_t> >::const_iterator it=bins.begin(); it!=bins.end(); ++it, n++)
	{
		fprintf(oh, "%u,%f,%lu\n", 1<<n, (double) it->first/it->second, (unsigned long) it->second);
	}
	fclose(oh);
	printf("Done!\n");
	return 0;
}
