/* test.cpp
An example of how to use nedalloc in C++
(C) 2010 Niall Douglas
*/

#define _CRT_SECURE_NO_WARNINGS 1	/* Don't care about MSVC warnings on POSIX functions */
#include <stdio.h>
#include <stdlib.h>
#include "nedmalloc.h"
#include <vector>
#if defined(_M_X64) || defined(__x86_64__) || (defined(_M_IX86) && _M_IX86_FP>=2) || (defined(__i386__) && defined(__SSE2__))
#include <emmintrin.h>
#endif

#ifdef _MSC_VER
/*#pragma optimize("g", off)*/	/* Useful for debugging */
#endif

#if !defined(USE_NEDMALLOC_DLL)
#include "nedmalloc.c"
#elif defined(WIN32)
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#ifdef _MSC_VER
#define MEMALIGNED(v) __declspec(align(v))
#elif defined(__GNUC__)
#define MEMALIGNED(v) __attribute__ ((aligned(v)))
#else
#define MEMALIGNED(v)
#endif

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
#include <time.h>
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

using namespace nedalloc;
using namespace std;

// Move constructors are utterly required to store aligned values in a STL collection
// In fact, alignment can't be guaranteed without them, so pre-C++0x you couldn't do this
struct
#ifdef HAVE_CPP0XRVALUEREFS
	MEMALIGNED(16)
#endif
SSEVectorType
{
	union {
#if defined(_M_X64) || defined(__x86_64__) || (defined(_M_IX86) && _M_IX86_FP>=2) || (defined(__i386__) && defined(__SSE2__))
		__m128i vec;
#endif
		struct {
			int i[4];
		} ints;
	} data;
	SSEVectorType() { }
	SSEVectorType(int a, int b, int c, int d) { data.ints.i[0]=a; data.ints.i[1]=b; data.ints.i[2]=c; data.ints.i[3]=d; }
	SSEVectorType(const SSEVectorType &) { /* do nothing */}
#ifdef HAVE_CPP0XRVALUEREFS
private:
	SSEVectorType &operator=(const SSEVectorType &);
public:
	SSEVectorType(SSEVectorType &&o)
	{
#if defined(_M_X64) || defined(__x86_64__) || (defined(_M_IX86) && _M_IX86_FP>=2) || (defined(__i386__) && defined(__SSE2__))
		vec=std::move(o.vec);
#else
		ints=std::move(o.ints);
#endif
	}
	SSEVectorType &&operator=(SSEVectorType &&o)
	{
#if defined(_M_X64) || defined(__x86_64__) || (defined(_M_IX86) && _M_IX86_FP>=2) || (defined(__i386__) && defined(__SSE2__))
		vec=std::move(o.vec);
#else
		ints=std::move(o.ints);
#endif
		return *this;
	}
#endif
	void checkaddr() const
	{
		size_t myaddr=(size_t) this;
		printf("SSEVectorType lives at %p and contains %u,%u,%u,%u\n", this, data.ints.i[0], data.ints.i[1], data.ints.i[2], data.ints.i[3]);
		if(myaddr & 15)
		{
			printf("SSEVectorType is not 16 byte aligned!\n");
			abort();
		}
	}
};

// This is an unsigned integer-ish type with a constructor
// to inhibit POD vector optimisations
struct UIntish
{
	unsigned int value;
	UIntish() : value(5) { }
	UIntish(const UIntish &o) : value(o.value) { }
#ifdef HAVE_CPP0XRVALUEREFS
	UIntish(UIntish &&o) : value(std::move(o)) { }
#endif
};

template<class vectype> usCount test(const char *desc)
{
	usCount time1=GetUsCount();
	vectype vec1(5000000), vec2(5000000);
	typename vectype::value_type v;
	for(int n=0; n<5000000; n++)
	{
		vec1.push_back(v);
		vec2.push_back(v);
	}
	usCount time2=GetUsCount();
	vec1.clear();
	usCount time3=GetUsCount();
	vec1=vec2;
	usCount time4=GetUsCount();
	vec2.insert(vec2.end(), vec1.begin(), vec1.end());
	usCount time5=GetUsCount();
	while(vec1.size()>1)
		vec1.pop_back();
	usCount time6=GetUsCount();
	size_t capacity=vec1.capacity()+vec2.capacity(), size=vec1.size()+vec2.size();
	printf("%s:\n"
		"    Appending each of 10,000,000 elements: %fms\n"
		"    Clearing 5,000,000 elements:           %fms\n"
		"    Assigning 5,000,000 elements:          %fms\n"
		"    Appending block of 5,000,000 elements: %fms\n"
		"    Popping 4,999,999 elements:            %fms\n"
		"    Overallocation wastage:                %f%%\n"
		"Total time: %fms\n\n", desc,
		(time2-time1)/1000000000.0, (time3-time2)/1000000000.0, (time4-time3)/1000000000.0, (time5-time4)/1000000000.0, (time6-time5)/1000000000.0,
		100.0*(capacity-size)/size,
		(time6-time1)/1000000000.0);
	return time6-time1;
}

int PatchInNedmallocDLL(void);
int main(void)
{
#if defined(WIN32) && defined(USE_NEDMALLOC_DLL)
	PatchInNedmallocDLL();
#endif

#ifdef WIN32
#pragma comment(lib, "user32.lib")
	{	/* Force load of user32.dll so we can debug */
		BOOL v;
		SystemParametersInfo(SPI_GETBEEP, 0, &v, 0);
	}
#endif

	{
		/* This is the classic usage scenario: simply give the
		STL collection class a nedallocator of the same type */
		vector<int, nedallocator<int> > anyvector1;


		/* What if we are allocating SSE/AVX vectors and we
		need the array always allocated on a 16 byte boundary? */
		printf("\nUninitialised (may contain random garbage):\n");
		vector<SSEVectorType, nedallocator<SSEVectorType, nedpolicy::align<16>::policy > > SSEvector1(5);
		for(vector<SSEVectorType, nedallocator<SSEVectorType, nedpolicy::align<16>::policy > >::const_iterator it=SSEvector1.begin(); it!=SSEvector1.end(); ++it)
			it->checkaddr();

		/* You can combine an arbitrary number of policies, so
		the following works as expected. Remember that policies
		are weakest to the right and strongest to the left, so
		leftmost policies always override rightmost policies.
		
		SSEVectorType doesn't initialise nor copy construct its
		contents in order to make it fast to copy and move around,
		but this means that on first instantiation it contains
		random data. Setting the nedpolicy::zero::policy fixes this. */
		printf("\nInitialised to zero:\n");
		typedef vector<SSEVectorType, nedallocator<SSEVectorType,
			nedpolicy::align<16>::policy,
			nedpolicy::zero<>::policy,
			nedpolicy::typeIsPOD<true>::policy
		> > SSEvector2Type;
		SSEvector2Type SSEvector2(5);
		for(SSEvector2Type::const_iterator it=SSEvector2.begin(); it!=SSEvector2.end(); ++it)
			it->checkaddr();


		/* What if you just want to allocate one of or a fixed sized
		array of some type? Sadly we can't use operator new because
		the C++ spec only allows one global operator delete, so
		instead we have New<type>(args...).

		<rant mode>THIS IS HOW operator new SHOULD HAVE BEEN
		IMPLEMENTED IN THE FIRST GOD DAMN PLACE!!!</rant mode>
		*/
		SSEVectorType *foo1=New<SSEVectorType>(4, 5, 6, 7);
		Delete(foo1);

		/* You needn't use nedallocator<> if you don't want, you
		can use ANY STL allocator implementation */
		SSEVectorType *foo2=New<SSEVectorType, std::allocator<SSEVectorType> >(4, 5, 6, 7);
		Delete<std::allocator<SSEVectorType> >(foo2);



		/* Here comes the real magic! Let us try comparing the
		speeds of various std::vector<> implementations using
		the UIntish type, an unsigned integer which pretends
		not to be POD */
		printf("\nSpeed test:\n");
		test<vector<unsigned int> >("vector<unsigned int>");
		test<vector<UIntish> >("vector<UIntish>");
		test<nedallocatorise<vector, UIntish,
			nedpolicy::typeIsPOD<true>::policy,
			nedpolicy::mmap<>::policy,
			nedpolicy::reserveN<26>::policy			// 1<<26 = 64Mb. 10,000,000 * sizeof(unsigned int) = 38Mb.
		>::value>("nedallocatorise<vector, UIntish, nedpolicy::typeIsPOD<true>>");

		printf("\nPress a key to trim\n");
		getchar();
		nedmalloc_trim(0);
#ifdef _MSC_VER
		printf("\nPress a key to end\n");
		getchar();
#endif
	}
	neddestroysyspool();
	return 0;
}
