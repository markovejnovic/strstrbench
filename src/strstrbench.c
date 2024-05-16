#include "ubench.h"
#include <stdlib.h>
#include <string.h>
#include "stringzilla/stringzilla.h"

#ifndef BENCH_NAME
#define BENCH_NAME sz_search
#endif

#ifndef BENCH_HAY_SZ
#define BENCH_HAY_SZ 128
#endif

#ifndef BENCH_NEEDLE_SZ
#define BENCH_NEEDLE_SZ 32
#endif

struct bench_fixture {
    char* haystack;
    size_t haystack_sz;

    char* needle;
    size_t needle_sz;
};

static void setup_fixture(struct bench_fixture* ubench_fixture,
                          size_t hay, size_t needle) {
    ubench_fixture->haystack_sz = hay;
    ubench_fixture->needle_sz = needle;

    ubench_fixture->haystack = malloc(ubench_fixture->haystack_sz);
    ubench_fixture->needle = malloc(ubench_fixture->needle_sz);
};

static void teardown_fixture(struct bench_fixture* ubench_fixture) {
    free(ubench_fixture->haystack);
    free(ubench_fixture->needle);
};

#define ASIZE 1024
#define XSIZE 2048
#define MAX(a, b) ((a) >= (b) ? (a) : (b))

void preBmBc(char *x, int m, int bmBc[]) {
   int i;
 
   for (i = 0; i < ASIZE; ++i)
      bmBc[i] = m;
   for (i = 0; i < m - 1; ++i)
      bmBc[x[i]] = m - i - 1;
}
 
 
void suffixes(char *x, int m, int *suff) {
   int f, g, i;
 
   suff[m - 1] = m;
   g = m - 1;
   for (i = m - 2; i >= 0; --i) {
      if (i > g && suff[i + m - 1 - f] < i - g)
         suff[i] = suff[i + m - 1 - f];
      else {
         if (i < g)
            g = i;
         f = i;
         while (g >= 0 && x[g] == x[g + m - 1 - f])
            --g;
         suff[i] = f - g;
      }
   }
}
 
void preBmGs(char *x, int m, int bmGs[]) {
   int i, j, suff[XSIZE];
 
   suffixes(x, m, suff);
 
   for (i = 0; i < m; ++i)
      bmGs[i] = m;
   j = 0;
   for (i = m - 1; i >= 0; --i)
      if (suff[i] == i + 1)
         for (; j < m - 1 - i; ++j)
            if (bmGs[j] == m)
               bmGs[j] = m - 1 - i;
   for (i = 0; i <= m - 2; ++i)
      bmGs[m - 1 - suff[i]] = m - 1 - i;
}
 
 
int BM(char *x, int m, char *y, int n) {
   int i, j, bmGs[XSIZE], bmBc[ASIZE];
 
   /* Preprocessing */
   preBmGs(x, m, bmGs);
   preBmBc(x, m, bmBc);
 
   /* Searching */
   j = 0;
   while (j <= n - m) {
      for (i = m - 1; i >= 0 && x[i] == y[i + j]; --i);
      if (i < 0) {
          return j;
         j += bmGs[0];
      }
      else
         j += MAX(bmGs[i], bmBc[y[i + j]] - m + 1 + i);
   }
}

#define BENCH_DECL(name, haysz, needlesz) \
UBENCH_EX(bench_fixture, name##_##haysz##_## needlesz ) { \
    struct bench_fixture* ubench_fixture = malloc(sizeof(struct bench_fixture)); \
\
    setup_fixture(ubench_fixture, haysz, needlesz); \
\
    UBENCH_DO_BENCHMARK() { \
        UBENCH_DO_NOTHING( \
            name (ubench_fixture->haystack, ubench_fixture->haystack_sz, \
                   ubench_fixture->needle, ubench_fixture->needle_sz) \
        ); \
    } \
\
    teardown_fixture(ubench_fixture); \
\
    free(ubench_fixture); \
}

BENCH_DECL(BENCH_NAME, BENCH_HAY_SZ, BENCH_NEEDLE_SZ);


UBENCH_MAIN();
