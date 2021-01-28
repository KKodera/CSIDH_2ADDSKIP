#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fp.h"
#include "mont.h"
#include "csidh.h"
#include "cycle.h"
#include "primes.h"

void isog_setup(proj *A,proj *P,proj *K,long long k)
{
  A->x = fp_0;
  A->z = fp_1;

  for (;;) {
    fp_random(&P->x);
    fp_random(&P->z);

    uintbig cof = uintbig_1;
    uintbig_mul3_64(&cof, &cof, 4);
    for (long long i = 0;i < primes_num;++i)
      if (primes[i] != k)
        uintbig_mul3_64(&cof, &cof, primes[i]);

    xMUL(K,A,P,&cof);

    if (memcmp(&K->z, &fp_0, sizeof(fp))) break;
  }

  uintbig cof = uintbig_1;
  uintbig_mul3_64(&cof, &cof, k);
  xMUL(P,A,K,&cof);
  if (memcmp(&P->z, &fp_0, sizeof(fp))) abort();

  fp_random(&P->x);
  fp_random(&P->z);
}

#define TIMINGS 15
#define KEYS 65

private_key priv[KEYS][TIMINGS];
public_key pub[KEYS][TIMINGS];
public_key pub1[KEYS][TIMINGS];

proj A[primes_num][TIMINGS];
proj P[primes_num][TIMINGS];
proj K[primes_num][TIMINGS];
proj A0[primes_num][TIMINGS];
proj P0[primes_num][TIMINGS];
proj K0[primes_num][TIMINGS];
proj A1[primes_num][TIMINGS];
proj P1[primes_num][TIMINGS];
proj K1[primes_num][TIMINGS];

long long bench_isogeny[2][primes_num][TIMINGS];
long long bench_action[2][KEYS][TIMINGS];
long long bench_validate[2][KEYS][TIMINGS];

int proj_equal(proj *A,proj *B)
{
  fp AxBz;
  fp AzBx;
  fp_mul3(&AxBz,&A->x,&B->z);
  fp_mul3(&AzBx,&A->z,&B->x);
  return !memcmp(&AxBz,&AzBx,sizeof AzBx);
}

int main()
{
  long long bench;
  bool ok;

  for (long long lpos = 0;lpos < primes_num;++lpos)
    for (long long t = 0;t < TIMINGS;++t)
      isog_setup(&A0[lpos][t],&P0[lpos][t],&K0[lpos][t],primes[lpos]);

  for (long long key = 0;key < KEYS;++key) {
    csidh_private(&priv[key][0]);
    for (long long t = 1;t < TIMINGS;++t)
      priv[key][t] = priv[key][0];
  }

  for (long long old = 0;old < 2;++old) {
    for (long long lpos = 0;lpos < primes_num;++lpos) {
      for (long long t = 0;t < TIMINGS;++t) {
        A[lpos][t] = A0[lpos][t];
        P[lpos][t] = P0[lpos][t];
        K[lpos][t] = K0[lpos][t];
      }
    }
    if (old)
      for (long long lpos = 0;lpos < primes_num;++lpos)
        for (long long t = 0;t < TIMINGS;++t) {
          bench = getticks();
          xISOG_old(&A[lpos][t],&P[lpos][t],&K[lpos][t],primes[lpos]);
          bench_isogeny[old][lpos][t] = getticks() - bench;
        }
    else
      for (long long lpos = 0;lpos < primes_num;++lpos)
        for (long long t = 0;t < TIMINGS;++t) {
          bench = getticks();
          xISOG(&A[lpos][t],&P[lpos][t],&K[lpos][t],primes[lpos]);
          bench_isogeny[old][lpos][t] = getticks() - bench;
        }
    for (long long lpos = 0;lpos < primes_num;++lpos)
      for (long long t = 0;t < TIMINGS;++t)
        if (!old) {
          A1[lpos][t] = A[lpos][t];
          P1[lpos][t] = P[lpos][t];
          K1[lpos][t] = K[lpos][t];
        } else {
          assert(proj_equal(&A1[lpos][t],&A[lpos][t]));
          assert(proj_equal(&P1[lpos][t],&P[lpos][t]));
          assert(proj_equal(&K1[lpos][t],&K[lpos][t]));
        }
  
    for (long long key = 0;key < KEYS;++key) {
      for (long long t = 0;t < TIMINGS;++t) {
        bench = getticks();
        action(&pub[key][t],&base,&priv[key][t]);
        bench_action[old][key][t] = getticks() - bench;

        bench = getticks();
        ok = validate(&pub[key][t]);
        bench_validate[old][key][t] = getticks() - bench;
        assert(ok);
      }
    }
    for (long long key = 0;key < KEYS;++key)
      for (long long t = 0;t < TIMINGS;++t)
        if (!old)
          pub1[key][t] = pub[key][t];
        else
          assert(!memcmp(&pub1[key][t],&pub[key][t],sizeof pub[key][t]));

    csidh_useoldxISOG();
  }

  for (long long lpos = 0;lpos < primes_num;++lpos) {
    printf("l %lld cycles",primes[lpos]);
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_isogeny[0][lpos][t]);
    printf(" baseline");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_isogeny[1][lpos][t]);
    printf("\n");
  }

  for (long long key = 0;key < KEYS;++key) {
    printf("action cycles");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_action[0][key][t]);
    printf(" baseline");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_action[1][key][t]);
    printf("\n");

    printf("validate cycles");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_validate[0][key][t]);
    printf(" baseline");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_validate[1][key][t]);
    printf("\n");
  }

  return 0;
}
