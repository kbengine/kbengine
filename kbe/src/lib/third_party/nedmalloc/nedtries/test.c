#include <stdio.h>
#include <string.h>
/*#define NEDTRIEUSEMACROS 1*/

#include "nedtrie.h"

#define RANDOM_NFIND_TEST_KEYMASK 63
#define RANDOM_NFIND_TEST_ITEMS ((RANDOM_NFIND_TEST_KEYMASK+1)/3)
#define ITERATIONS (1<<16)

/* Include the Mersenne twister */
#if !defined(__cplusplus_cli) && (defined(_M_X64) || defined(__x86_64__) || (defined(_M_IX86) && _M_IX86_FP>=2) || (defined(__i386__) && defined(__SSE2__)))
#define HAVE_SSE2 1
#endif
#define MEXP 19937
#include "SFMT.c"


typedef struct foo_s foo_t;
struct foo_s {
  NEDTRIE_ENTRY(foo_s) link;
  size_t key;
};
typedef struct foo_tree_s foo_tree_t;
NEDTRIE_HEAD(foo_tree_s, foo_s);
static foo_tree_t footree;

size_t fookeyfunct(const foo_t *r)
{
  return r->key;
}

NEDTRIE_GENERATE(static, foo_tree_s, foo_s, link, fookeyfunct, NEDTRIE_NOBBLEZEROS(foo_tree_s))

#ifdef __cplusplus
struct keyfunct : public std::unary_function<int, size_t>
{
    size_t operator()(const int &) const
    {
    return 5;
    }
};
#endif

int main(void)
{
  foo_t a, b, c, *r;

  printf("General workout of the C API ...\n");
  NEDTRIE_INIT(&footree);
  a.key=2;
  NEDTRIE_INSERT(foo_tree_s, &footree, &a);
  b.key=6;
  NEDTRIE_INSERT(foo_tree_s, &footree, &b);
  r=NEDTRIE_FIND(foo_tree_s, &footree, &b);
  assert(r==&b);
  c.key=5;
  r=NEDTRIE_NFIND(foo_tree_s, &footree, &c);
  assert(r==&b); /* NFIND finds next largest. Invert the key function (i.e. 1-key) to find next smallest. */
  NEDTRIE_REMOVE(foo_tree_s, &footree, &a);
  NEDTRIE_FOREACH(r, foo_tree_s, &footree)
  {
    printf("%p, %u\n", (void *) r, (unsigned) r->key);
  }
  assert(!NEDTRIE_PREV(foo_tree_s, &footree, &b));
  assert(!NEDTRIE_NEXT(foo_tree_s, &footree, &b));

#if defined(__cplusplus) && 1
  printf("General workout of the C++ API ...\n");
  assert(keyfunct()(78)==5);
  using namespace nedtries;
  trie_map<size_t, int, keyfunct> map;
  trie_multimap<size_t, int, keyfunct> multimap;
  map.insert(78);
  multimap.insert(78);
  map.insert(79); // Replaces 78 with 79
  multimap.insert(79); // Pushes the existing 78 backwards so 79 is now in front, so it's LIFO
  assert(map.size()==1);
  assert(multimap.size()==2);
  assert(79==*map.find(5));
  trie_multimap<size_t, int, keyfunct>::const_iterator it=multimap.find(5);
  assert(79==*it);
  --it; // NEDTRIE_PREV
  assert(78==*it);
#endif

  /* From https://github.com/ned14/nedtries/issues/5 */
  printf("Testing bug from https://github.com/ned14/nedtries/issues/5 (Nfind does not work in all cases) ...\n");
  NEDTRIE_INIT(&footree);
  a.key=0x10; /* 16 = 10000 */
  NEDTRIE_INSERT(foo_tree_s, &footree, &a);
  b.key=0x18; /* 24 = 11000 */
  NEDTRIE_INSERT(foo_tree_s, &footree, &b);
  c.key=0x11; /* 17 = 10001 */
  r=NEDTRIE_NFIND(foo_tree_s, &footree, &c);
  assert(r==&b); /* This was failing under the bug */

  NEDTRIE_INIT(&footree);
  a.key=52; /* 110100 */
  NEDTRIE_INSERT(foo_tree_s, &footree, &a);
  b.key=40; /* 101000 */
  NEDTRIE_INSERT(foo_tree_s, &footree, &b);
  c.key=25; /* 011001 */
  r=NEDTRIE_NFIND(foo_tree_s, &footree, &c);
  assert(r==&b); /* This was failing under the bug */

  /* To make sure the above never happens again */
  init_gen_rand(1234);
  {
    int n, m, promoted=0;
    foo_t items[RANDOM_NFIND_TEST_ITEMS];
    const foo_t *items_sorted[RANDOM_NFIND_TEST_ITEMS];
    foo_t *r2;
    for(n=0; n<ITERATIONS; n++)
    {
      const foo_t *max, *min;
      int sorted;
      NEDTRIE_INIT(&footree);
      for(m=0; m<RANDOM_NFIND_TEST_ITEMS; m++)
      {
        int o;
        do
        {
          items[m].key=gen_rand32() & RANDOM_NFIND_TEST_KEYMASK;
          for(o=0; o<m && items[o].key!=items[m].key; o++);
        } while(o<m);
        items_sorted[m]=&items[m];
        NEDTRIE_INSERT(foo_tree_s, &footree, &items[m]);
      }
      m=0;
      NEDTRIE_FOREACH(r2, foo_tree_s, &footree)
      {
        m++;
      }
      assert(m==RANDOM_NFIND_TEST_ITEMS);
      m=0;
      NEDTRIE_FOREACH_REVERSE(r2, foo_tree_s, &footree)
      {
        m++;
      }
      assert(m==RANDOM_NFIND_TEST_ITEMS);
      do
      {
        sorted=1;
        for(m=0; m<RANDOM_NFIND_TEST_ITEMS-1; m++)
        {
          if(items_sorted[m+1]->key<items_sorted[m]->key)
          {
            const foo_t *temp;
            sorted=0;
            temp=items_sorted[m];
            items_sorted[m]=items_sorted[m+1];
            items_sorted[m+1]=temp;
          }
        }
      } while(!sorted);
      min=items_sorted[0];
      max=items_sorted[RANDOM_NFIND_TEST_ITEMS-1];

      c.key=gen_rand32() & RANDOM_NFIND_TEST_KEYMASK;
      r2=NEDTRIE_CFIND(foo_tree_s, &footree, &c, INT_MAX);
      r=NEDTRIE_NFIND(foo_tree_s, &footree, &c);
      promoted+=(r!=r2);
      if(c.key>max->key)
      { /* If search key is bigger than max key, it must always return zero */
        assert(r==0);
      }
      else
      {
        assert(r!=0);
        if(c.key!=r->key)
        { /* If search key is smaller than min key, it must always return min */
          if(c.key<min->key)
          {
            assert(r==min);
          }
          else
          { /* Assert it correctly clamped to nearest */
            for(m=0; m<RANDOM_NFIND_TEST_ITEMS && items_sorted[m]->key<c.key; m++);
            assert(m!=RANDOM_NFIND_TEST_ITEMS);
            assert(r==items_sorted[m]);
          }
        }
      }
    }
    printf("Nfind returned a different item to Cfind %d of %d (%f%%) iterations\n", promoted, ITERATIONS, 100.0*promoted/ITERATIONS);
    printf("\nPress Return to exit ...\n");
    getchar();
  }
  return 0;
}
