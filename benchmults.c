#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fp.h"
#include "mont.h"
#include "csidh.h"
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

#define TIMINGS 7
#define KEYS 65

private_key priv[KEYS][TIMINGS];
public_key pub[KEYS][TIMINGS];
public_key pub1[KEYS][TIMINGS];

proj A[primes_num];
proj P[primes_num];
proj K[primes_num];
proj A0[primes_num];
proj P0[primes_num];
proj K0[primes_num];
proj A1[primes_num];
proj P1[primes_num];
proj K1[primes_num];

long long bench_isogeny[3][primes_num];
long long bench_action[3][KEYS][TIMINGS];
long long bench_validate[3][KEYS][TIMINGS];

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
    isog_setup(&A0[lpos],&P0[lpos],&K0[lpos],primes[lpos]);

  for (long long key = 0;key < KEYS;++key) {
    csidh_private(&priv[key][0]);
    for (long long t = 1;t < TIMINGS;++t)
      priv[key][t] = priv[key][0];
  }

  for (long long type = 0;type < 3;++type) {
    for (long long lpos = 0;lpos < primes_num;++lpos) {
      A[lpos] = A0[lpos];
      P[lpos] = P0[lpos];
      K[lpos] = K0[lpos];
    }
    if (type == 0) {
      for (long long lpos = 0;lpos < primes_num;++lpos) {
        bench = fp_mul_count;
        xISOG(&A[lpos],&P[lpos],&K[lpos],primes[lpos]);
        bench_isogeny[type][lpos] = fp_mul_count - bench;
      }
    }
    else if (type == 1) {
      for (long long lpos = 0;lpos < primes_num;++lpos) {
        bench = fp_mul_count;
        xISOG_old(&A[lpos],&P[lpos],&K[lpos],primes[lpos]);
        bench_isogeny[type][lpos] = fp_mul_count - bench;
      }
    } else {
      for (long long lpos = 0;lpos < primes_num;++lpos) {
        bench = fp_mul_count;
        xISOG_2ADDSkip(&A[lpos],&P[lpos],&K[lpos],primes[lpos], nfor2addskip[lpos]);
        bench_isogeny[type][lpos] = fp_mul_count - bench;
      }
    }
    for (long long lpos = 0;lpos < primes_num;++lpos)
      if (type == 0) {
        A1[lpos] = A[lpos];
        P1[lpos] = P[lpos];
        K1[lpos] = K[lpos];
      } else {
        assert(proj_equal(&A1[lpos],&A[lpos]));
        assert(proj_equal(&P1[lpos],&P[lpos]));
        assert(proj_equal(&K1[lpos],&K[lpos]));
      }
  
    for (long long key = 0;key < KEYS;++key)
      for (long long t = 0;t < TIMINGS;++t) {
        bench = fp_mul_count;
        action(&pub[key][t],&base,&priv[key][t]);
        bench_action[type][key][t] = fp_mul_count - bench;

        bench = fp_mul_count;
        ok = validate(&pub[key][t]);
        bench_validate[type][key][t] = fp_mul_count - bench;
        assert(ok);
      }
    for (long long key = 0;key < KEYS;++key)
      for (long long t = 0;t < TIMINGS;++t)
        if (type == 0)
          pub1[key][t] = pub[key][t];
        else
          assert(!memcmp(&pub1[key][t],&pub[key][t],sizeof pub[key][t]));
    if (type == 0)
      csidh_useoldxISOG();
    else
      csidh_usexISOG_2ADDSkip();
  }

  for (long long lpos = 0;lpos < primes_num;++lpos)
    printf("l %lld mults %lld baseline %lld 2addskip %lld\n",primes[lpos],bench_isogeny[0][lpos],bench_isogeny[1][lpos],bench_isogeny[2][lpos]);

  for (long long key = 0;key < KEYS;++key) {
    printf("action mults");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_action[0][key][t]);
    printf(" baseline");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_action[1][key][t]);
    printf(" 2addskip");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_action[2][key][t]);
    printf("\n");

    printf("validate mults");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_validate[0][key][t]);
    printf(" baseline");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_validate[1][key][t]);
    printf(" 2addskip");
    for (long long t = 0;t < TIMINGS;++t)
      printf(" %lld",bench_validate[2][key][t]);
    printf("\n");
  }

  return 0;
}
