/* Included by benchmark.cpp - has same licence as it */

typedef struct BENCHMARK_PREFIX(region_node_s) BENCHMARK_PREFIX(region_node_t);
struct BENCHMARK_PREFIX(region_node_s) {
  REGION_ENTRY(BENCHMARK_PREFIX(region_node_s)) link;
  size_t key;
};
#ifndef BENCHMARK_NOHEADTYPE
typedef struct BENCHMARK_PREFIX(region_tree_s) BENCHMARK_PREFIX(region_tree_t);
REGION_HEAD(BENCHMARK_PREFIX(region_tree_s), BENCHMARK_PREFIX(region_node_s));
#else
REGION_HEAD(BENCHMARK_PREFIX(region_tree_s), BENCHMARK_PREFIX(region_node_s));
typedef struct BENCHMARK_PREFIX(region_node_s) *BENCHMARK_PREFIX(region_tree_t);
#endif

#ifdef BENCHMARK_USEKEYFUNCT
size_t BENCHMARK_PREFIX(regionFunct)(const BENCHMARK_PREFIX(region_node_t) *a)
{
  return a->key;
}
#else
int BENCHMARK_PREFIX(regionFunct)(const BENCHMARK_PREFIX(region_node_t) *a, const BENCHMARK_PREFIX(region_node_t) *b)
{
  return (a->key > b->key) - (a->key < b->key);
}
#endif

REGION_GENERATE(static, BENCHMARK_PREFIX(region_tree_s), BENCHMARK_PREFIX(region_node_s), link, BENCHMARK_PREFIX(regionFunct));

static BENCHMARK_PREFIX(region_tree_t) BENCHMARK_PREFIX(regiontree);

typedef struct BENCHMARK_PREFIX(region_node2_s)
{
  BENCHMARK_PREFIX(region_node_t) node;
#if ITEMSIZE > 0
  char padding[ITEMSIZE<=sizeof(BENCHMARK_PREFIX(region_node_t)) ? 1 : (ITEMSIZE-sizeof(BENCHMARK_PREFIX(region_node_t)))];
#endif
} BENCHMARK_PREFIX(region_node2_t);
static void BENCHMARK_PREFIX(RunTest)(AlgorithmInfo *ai)
{
  static BENCHMARK_PREFIX(region_node2_t) nodes[1<<ALLOCATIONS];
  BENCHMARK_PREFIX(region_node_t) *r;
  int l, n, m;
  usCount start, end;
  printf("\nRunning scalability test for %s\n", ai->name);
  printf("sizeof(REGION_ENTRY)=%d\n", (int) sizeof(nodes[0].node.link));
  init_gen_rand(1234);
  REGION_INIT(&BENCHMARK_PREFIX(regiontree));
  for(n=0; n<(1<<ALLOCATIONS); n++)
  {
  tryagain:
    nodes[n].node.key=gen_rand32();
    r=REGION_FIND(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &(nodes+n)->node);
    if(!r)
    {
      REGION_INSERT(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &(nodes+n)->node);
    }
    else goto tryagain;
  }
  REGION_INIT(&BENCHMARK_PREFIX(regiontree));
  for(m=0; m<ALLOCATIONS; m++)
  {
    usCount insert=0, find1=0, find2=0, remove=0, iterate=0, nfind=0, cfind1=0, cfind2=0;
    int lmax=(ALLOCATIONS*ALLOCATIONS*8-(m*m*m*m)); /* Loop more when m is smaller */
    lmax*=AVERAGE;
    if(lmax<1) lmax=1;
    printf("Nodes=%d, iterations=%d\n", 1<<m, lmax);
    for(l=0; l<lmax; l++)
    {
      for(n=0; n<(1<<m); n++)
      {
        int ridx=gen_rand32() % (n+1);
        start=GetUsCount();
        REGION_INSERT(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &(nodes+n)->node);
        end=GetUsCount();
        insert+=end-start-usCountOverhead;
        start=GetUsCount();
        r=REGION_FIND(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &(nodes+ridx)->node);
        end=GetUsCount();
        if(!r) abort();
        find1+=end-start-usCountOverhead;
      }
      for(n=0; n<(1<<m); n++)
      {
        start=GetUsCount();
        r=REGION_FIND(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &(nodes+n)->node);
        end=GetUsCount();
        if(!r) abort();
        find2+=end-start-usCountOverhead;
      }
#ifdef REGION_CFIND1
      for(n=0; n<(1<<m); n++)
      {
        BENCHMARK_PREFIX(region_node_t) t;
        t.key=gen_rand32();
        start=GetUsCount();
        r=REGION_CFIND1(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &t);
        end=GetUsCount();
        cfind1+=end-start-usCountOverhead;
      }
#endif
#ifdef REGION_CFIND2
      if(m<=18)
      {
        for(n=0; n<(1<<m); n++)
        {
          BENCHMARK_PREFIX(region_node_t) t;
          t.key=gen_rand32();
          start=GetUsCount();
          r=REGION_CFIND2(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &t);
          end=GetUsCount();
          cfind2+=end-start-usCountOverhead;
        }
      }
#endif
#ifdef REGION_NFIND
      if(m<=15)
      {
        for(n=0; n<(1<<m); n++)
        {
          BENCHMARK_PREFIX(region_node_t) t;
          t.key=gen_rand32();
          start=GetUsCount();
          r=REGION_NFIND(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &t);
          end=GetUsCount();
          nfind+=end-start-usCountOverhead;
        }
      }
#endif
      for(r=REGION_MIN(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree)); r;)
      {
        start=GetUsCount();
        r=REGION_NEXT(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), r);
        end=GetUsCount();
        iterate+=end-start-usCountOverhead;
      }
      for(n=0; n<(1<<m); n++)
      {
        start=GetUsCount();
        REGION_REMOVE(BENCHMARK_PREFIX(region_tree_s), &BENCHMARK_PREFIX(regiontree), &(nodes+n)->node);
        end=GetUsCount();
        remove+=end-start-usCountOverhead;
      }
    }
    ai->inserts[m]=(usCount)((double) insert/l);
    ai->finds1[m]=(usCount)((double)find1/l);
    ai->finds2[m]=(usCount)((double)find2/l);
    ai->removes[m]=(usCount)((double)remove/l);
    ai->iterates[m]=(usCount)((double)iterate/l);
    ai->cfind1s[m]=(usCount)((double)cfind1/l);
    ai->cfind2s[m]=(usCount)((double)cfind2/l);
    ai->nfinds[m]=(usCount)((double)nfind/l);
    /*if(!(m & 127)) printf("At %d = %llu, %llu, %llu, %llu, %llu\n", m, ai->inserts[m], ai->finds1[m], ai->finds2[m], ai->removes[m], ai->iterates[m]);*/
  }
}