#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "steps.h"
#include "mont.h"
#include "uintbig.h"
#include "poly.h"

void xDBLADD(proj *R, proj *S, proj const *P, proj const *Q, proj const *PQ, proj const *A24)
{
    fp tmp0, tmp1, tmp2;        //requires precomputation of A24=(A+2C:4C)

    fp_add3(&tmp0, &P->x, &P->z);
    fp_sub3(&tmp1, &P->x, &P->z);
    fp_sq2(&R->x, &tmp0);
    fp_sub3(&tmp2, &Q->x, &Q->z);
    fp_add3(&S->x, &Q->x, &Q->z);
    fp_mul2(&tmp0, &tmp2);
    fp_sq2(&R->z, &tmp1);
    fp_mul2(&tmp1, &S->x);
    fp_sub3(&tmp2, &R->x, &R->z);
    fp_mul2(&R->z, &A24->z);
    fp_mul2(&R->x, &R->z);
    fp_mul3(&S->x, &A24->x, &tmp2);
    fp_sub3(&S->z, &tmp0, &tmp1);
    fp_add2(&R->z, &S->x);
    fp_add3(&S->x, &tmp0, &tmp1);
    fp_mul2(&R->z, &tmp2);
    fp_sq1(&S->z);
    fp_sq1(&S->x);
    fp_mul2(&S->z, &PQ->x);
    fp_mul2(&S->x, &PQ->z);
}

void xDBL(proj *Q, proj const *A, proj const *P)
{
    fp a, b, c;
    fp_add3(&a, &P->x, &P->z);
    fp_sq1(&a);
    fp_sub3(&b, &P->x, &P->z);
    fp_sq1(&b);
    fp_sub3(&c, &a, &b);
    fp_add2(&b, &b); fp_add2(&b, &b); /* multiplication by 4 */
    fp_mul2(&b, &A->z);
    fp_mul3(&Q->x, &a, &b);
    fp_add3(&a, &A->z, &A->z); /* multiplication by 2 */
    fp_add2(&a, &A->x);
    fp_mul2(&a, &c);
    fp_add2(&a, &b);
    fp_mul3(&Q->z, &a, &c);
}

void xADD(proj *S, proj const *P, proj const *Q, proj const *PQ)
{
    fp a, b, c, d;
    fp_add3(&a, &P->x, &P->z);
    fp_sub3(&b, &P->x, &P->z);
    fp_add3(&c, &Q->x, &Q->z);
    fp_sub3(&d, &Q->x, &Q->z);
    fp_mul2(&a, &d);
    fp_mul2(&b, &c);
    fp_add3(&c, &a, &b);
    fp_sub3(&d, &a, &b);
    fp_sq1(&c);
    fp_sq1(&d);
    fp_mul3(&S->x, &PQ->z, &c);
    fp_mul3(&S->z, &PQ->x, &d);
}

/* Montgomery ladder. */
/* P must not be the unique point of order 2. */
/* not constant-time! */
void xMUL(proj *Q, proj const *A, proj const *P, uintbig const *k)
{
    proj R = *P;
    proj A24;
    const proj Pcopy = *P; /* in case Q = P */

    Q->x = fp_1;
    Q->z = fp_0;

    fp_add3(&A24.x, &A->z, &A->z);    //precomputation of A24=(A+2C:4C)
    fp_add3(&A24.z, &A24.x, &A24.x);
    fp_add2(&A24.x, &A->x);

    unsigned long i = BITS;
    while (--i && !uintbig_bit(k, i));

    do {

        bool bit = uintbig_bit(k, i);

        if (bit) { proj T = *Q; *Q = R; R = T; } /* not constant-time */
        //fp_cswap(&Q->x, &R.x, bit);
        //fp_cswap(&Q->z, &R.z, bit);

        xDBLADD(Q, &R, Q, &R, &Pcopy, &A24);

        if (bit) { proj T = *Q; *Q = R; R = T; } /* not constant-time */
        //fp_cswap(&Q->x, &R.x, bit);
        //fp_cswap(&Q->z, &R.z, bit);

    } while (i--);
}

//simultaneous square-and-multiply, computes x^exp and y^exp 
void exp_by_squaring_(fp* x, fp* y, uint64_t exp)
{
        fp result1, result2;
        fp_set(&result1, 1);
        fp_set(&result2, 1);

    while (exp)
    {
        if (exp & 1){
          fp_mul2(&result1, x);
          fp_mul2(&result2, y);
        }
        
        fp_sq1(x);
        fp_sq1(y);
        exp >>= 1;
    }

    fp_cswap(&result1, x, 1);
    fp_cswap(&result2, y, 1);

}

/* goal: compute coeffs of Z^0,Z^1,Z^2 in the poly */
/* F0(Z,P)Q^2 + F1(Z,P)Q + F2(Z,P) */
/* = (Z-P)^2*Q^2 - 2*((Z*P+1)*(Z+P)+2*A*Z*P)*Q + (Z*P-1)^2 */
/* = (P-Q)^2*Z^2 - 2*((Q*P+1)*(Q+P)+2*A*Q*P)*Z + (Q*P-1)^2 */
/* but multiply by denominators to avoid divisions: */
/* out[2] = Az*Pz^2*Qz^2*(P-Q)^2 */
/* out[1] = Az*Pz^2*Qz^2*-2*((Q*P+1)*(Q+P)+2*A*Q*P) */
/* out[0] = Az*Pz^2*Qz^2*(Q*P-1)^2 */
/* i.e.: */
/* out[2] = Az*(Px*Qz-Qx*Pz)^2 */
/* out[1] = -2*(Az*(Qx*Px+Qz*Pz)*(Qx*Pz+Px*Qz)+2*Ax*Qx*Qz*Px*Pz) */
/* out[0] = Az*(Qx*Px-Qz*Pz)^2 */
void biquad(fp *out,const proj *P,const proj *Q,const proj *A)
{
  fp PxQx; fp_mul3(&PxQx,&P->x,&Q->x);
  fp PxQz; fp_mul3(&PxQz,&P->x,&Q->z);
  fp PzQx; fp_mul3(&PzQx,&P->z,&Q->x);
  fp PzQz; fp_mul3(&PzQz,&P->z,&Q->z);

  fp s; fp_add3(&s,&PxQx,&PzQz);
  fp t; fp_add3(&t,&PzQx,&PxQz);
  fp_mul3(&out[1],&s,&t);
  fp_mul2(&out[1],&A->z);
  fp u; fp_mul3(&u,&PxQx,&PzQz);
  fp_mul2(&u,&A->x);
  fp_add2(&u,&u);
  fp_add2(&out[1],&u);
  fp_add2(&out[1],&out[1]);
  fp_neg1(&out[1]); /* XXX: push through other computations? */

  fp_sub3(&out[2],&PxQz,&PzQx);
  fp_sq1(&out[2]);
  fp_mul2(&out[2],&A->z);

  fp_sub3(&out[0],&PxQx,&PzQz);
  fp_sq1(&out[0]);
  fp_mul2(&out[0],&A->z);
}

/* same as biquad but with Q inverted */
void biquad_inv(fp *out,const proj *P,const proj *Q,const proj *A)
{
  fp PxQz; fp_mul3(&PxQz,&P->x,&Q->z);
  fp PxQx; fp_mul3(&PxQx,&P->x,&Q->x);
  fp PzQz; fp_mul3(&PzQz,&P->z,&Q->z);
  fp PzQx; fp_mul3(&PzQx,&P->z,&Q->x);

  fp s; fp_add3(&s,&PxQz,&PzQx);
  fp t; fp_add3(&t,&PzQz,&PxQx);
  fp_mul3(&out[1],&s,&t);
  fp_mul2(&out[1],&A->z);
  fp u; fp_mul3(&u,&PxQz,&PzQx);
  fp_mul2(&u,&A->x);
  fp_add2(&u,&u);
  fp_add2(&out[1],&u);
  fp_add2(&out[1],&out[1]);
  fp_neg1(&out[1]); /* XXX: push through other computations? */

  fp_sub3(&out[2],&PxQx,&PzQz);
  fp_sq1(&out[2]);
  fp_mul2(&out[2],&A->z);

  fp_sub3(&out[0],&PxQz,&PzQx);
  fp_sq1(&out[0]);
  fp_mul2(&out[0],&A->z);
}

/* biquad and biquad_inv */
void biquad_both(fp *out,fp *outinv,const proj *P,const proj *Q,const proj *A)
{
  fp PxQx; fp_mul3(&PxQx,&P->x,&Q->x);
  fp PxQz; fp_mul3(&PxQz,&P->x,&Q->z);
  fp PzQx; fp_mul3(&PzQx,&P->z,&Q->x);
  fp PzQz; fp_mul3(&PzQz,&P->z,&Q->z);
  fp PPQQ; fp_mul3(&PPQQ,&PxQx,&PzQz);
  fp_add2(&PPQQ,&PPQQ);
  fp_mul2(&PPQQ,&A->x);

  fp s,t;

  fp_add3(&s,&PxQx,&PzQz);
  fp_add3(&t,&PzQx,&PxQz);
  fp_mul3(&out[1],&s,&t);
  fp_mul2(&out[1],&A->z);
  fp_add2(&out[1],&PPQQ);
  fp_add2(&out[1],&out[1]);
  fp_neg1(&out[1]); /* XXX: push through other computations? */

  fp_sub3(&out[2],&PxQz,&PzQx);
  fp_sq1(&out[2]);
  fp_mul2(&out[2],&A->z);

  fp_sub3(&out[0],&PxQx,&PzQz);
  fp_sq1(&out[0]);
  fp_mul2(&out[0],&A->z);

  /* ----- */

  fp_add3(&s,&PxQz,&PzQx);
  fp_add3(&t,&PzQz,&PxQx);
  fp_mul3(&outinv[1],&s,&t);
  fp_mul2(&outinv[1],&A->z);
  fp_add2(&outinv[1],&PPQQ);
  fp_add2(&outinv[1],&outinv[1]);
  fp_neg1(&outinv[1]); /* XXX: push through other computations? */

  outinv[2] = out[0];
  outinv[0] = out[2];
}

/* biquad specifically for Q=1 */
/* i.e.: */
/* out[1] = -2*(Az*(Px+Pz)^2+2*Ax*Px*Pz) */
/* out[2] = out[0] = Az*(Px-Pz)^2 */
void biquad_1(fp *out,const proj *P,const proj *A)
{
  fp Pplus; fp_add3(&Pplus,&P->x,&P->z);
  fp Pminus; fp_sub3(&Pminus,&P->x,&P->z);
  fp_sq1(&Pplus);
  fp_sq1(&Pminus);

  fp_mul3(&out[0],&Pminus,&A->z);

  out[2] = out[0];

  fp_sub3(&out[1],&Pminus,&Pplus);
  fp_mul2(&out[1],&A->x);

  fp t; fp_mul3(&t,&Pplus,&A->z);
  fp_add2(&t,&t);
  fp_sub2(&out[1],&t);

/*
  proj Q;
  Q.x = fp_1;
  Q.z = fp_1;
  fp outcheck[3];
  biquad(outcheck,P,&Q,A);
  assert(!memcmp(out,&outcheck,3*sizeof(fp)));
*/
}

/* biquad specifically for Q=-1 */
/* i.e.: */
/* out[1] = 2*(Az*(Px-Pz)^2+2*Ax*Px*Pz) */
/* out[2] = out[0] = Az*(Px+Pz)^2 */
void biquad_minus1(fp *out,const proj *P,const proj *A)
{
  fp Pplus; fp_add3(&Pplus,&P->x,&P->z);
  fp Pminus; fp_sub3(&Pminus,&P->x,&P->z);
  fp_sq1(&Pplus);
  fp_sq1(&Pminus);

  fp_mul3(&out[0],&Pplus,&A->z);

  out[2] = out[0];

  fp_sub3(&out[1],&Pplus,&Pminus);
  fp_mul2(&out[1],&A->x);

  fp t; fp_mul3(&t,&Pminus,&A->z);
  fp_add2(&t,&t);
  fp_add2(&out[1],&t);

/*
  proj Q;
  Q.z = fp_1;
  fp_sub3(&Q.x,&fp_0,&Q.z);
  fp outcheck[3];
  biquad(outcheck,P,&Q,A);
  assert(!memcmp(out,&outcheck,3*sizeof(fp)));
*/
}

/* biquad_1 and biquad_minus1 */
void biquad_pm1(fp *outplus,fp *outminus,const proj *P,const proj *A)
{
  fp Pplus; fp_add3(&Pplus,&P->x,&P->z);
  fp Pminus; fp_sub3(&Pminus,&P->x,&P->z);
  fp_sq1(&Pplus);
  fp_sq1(&Pminus);

  fp_mul3(&outplus[0],&Pminus,&A->z);
  outplus[2] = outplus[0];
  fp_mul3(&outminus[0],&Pplus,&A->z);
  outminus[2] = outminus[0];

  fp u;
  fp_sub3(&u,&Pminus,&Pplus);
  fp_mul2(&u,&A->x);

  fp t;
  fp_add3(&t,&outminus[0],&outminus[0]);
  fp_sub3(&outplus[1],&u,&t);

  fp_add3(&t,&outplus[0],&outplus[0]);
  fp_sub3(&outminus[1],&t,&u);
}


/* computes the isogeny with kernel point K of order k */
/* returns the new curve coefficient A and the image of P */
/* (obviously) not constant time in k */
void xISOG(proj *A, proj *P, proj const *K, long long k)
{
    assert (k >= 3);
    assert (k % 2 == 1);

    long long sqrtvelu = 0;
    long long bs = 0;
    long long gs = 0;

    steps(&bs,&gs,k);
    if (bs) {
      sqrtvelu = 1;
      assert(bs > 0);
      assert(gs > 0);
      assert(!(bs&1));
    }

    fp tmp0, tmp1, tmp2;

    proj Aed;
    fp_add3(&Aed.z, &A->z, &A->z);  //compute twisted Edwards curve coefficients
    fp_add3(&Aed.x, &A->x, &Aed.z);
    fp_sub3(&Aed.z, &A->x, &Aed.z);
   
    fp Psum, Pdif;
    fp_add3(&Psum, &P->x, &P->z);   //precomputations
    fp_sub3(&Pdif, &P->x, &P->z);

    int Minit[k];
    proj M[k]; /* XXX: use less space */

    for (long long s = 0;s < k;++s) Minit[s] = 0;

    M[1] = *K; Minit[1] = 1;
    xDBL(&M[2], A, K); Minit[2] = 1;

    if (sqrtvelu) {
      for (long long s = 3;s < k;++s) {
        if (s & 1) {
          long long i = s/2;
          assert(s == 2*i+1);
          if (i < bs) {
            if (s == 3) {
              assert(Minit[1]);
              assert(Minit[2]);
              xADD(&M[s],&M[2],&M[1],&M[1]);
              Minit[s] = 1;
              continue;
            }
            assert(Minit[s-2]);
            assert(Minit[s-4]);
            assert(Minit[2]);
            xADD(&M[s],&M[s-2],&M[2],&M[s-4]);
            Minit[s] = 1;
            continue;
          }
        } else {
          long long i = s/2-1;
          assert(s == 2*i+2);
          if (i < (k-1)/2-2*bs*gs) {
            if (s == 4) {
              assert(Minit[2]);
              xDBL(&M[s],A,&M[2]);
              Minit[s] = 1;
              continue;
            }
            assert(Minit[s-2]);
            assert(Minit[s-4]);
            assert(Minit[2]);
            xADD(&M[s],&M[s-2],&M[2],&M[s-4]);
            Minit[s] = 1;
            continue;
          }
        }
  
        if (bs > 0) {
          if (s == 2*bs) {
            assert(Minit[bs-1]);
            assert(Minit[bs+1]);
            assert(Minit[2]);
            xADD(&M[s],&M[bs+1],&M[bs-1],&M[2]);
            Minit[s] = 1;
            continue;
          } else if (s == 4*bs) {
            assert(Minit[2*bs]);
            xDBL(&M[s],A,&M[2*bs]);
            Minit[s] = 1;
            continue;
          } else if (s == 6*bs) {
            assert(Minit[2*bs]);
            assert(Minit[4*bs]);
            xADD(&M[s],&M[4*bs],&M[2*bs],&M[2*bs]);
            Minit[s] = 1;
            continue;
          } else if (s%(4*bs) == 2*bs) {
            long long j = s/(4*bs);
            assert(s == 2*bs*(2*j+1));
            if (j < gs) {
              assert(Minit[s-4*bs]);
              assert(Minit[s-8*bs]);
              assert(Minit[4*bs]);
              xADD(&M[s],&M[s-4*bs],&M[4*bs],&M[s-8*bs]);
              Minit[s] = 1;
              continue;
            }
          }
        }
      }
    } else {
      for (long long i = 3;i <= (k-1)/2;++i) {
	Minit[i] = 1;
        xADD(&M[i],&M[i-1],K,&M[i-2]);
      }
    }

    proj Abatch;
    Abatch.x = fp_1;
    Abatch.z = fp_1;
    proj Qbatch;
    Qbatch.x = fp_1;
    Qbatch.z = fp_1;

    if (sqrtvelu) {
      long long TIlen = 2*bs+poly_tree1size(bs);
      fp TI[TIlen];
  
      for (long long i = 0;i < bs;++i) {
        assert(Minit[2*i+1]);
        fp_neg2(&TI[2*i],&M[2*i+1].x);
        TI[2*i+1] = M[2*i+1].z;
      }
  
      poly_tree1(TI+2*bs,TI,bs);
  
      fp TP[3*gs];
      fp TPinv[3*gs];
      fp T1[3*gs];
      fp Tminus1[3*gs];
  
      for (long long j = 0;j < gs;++j) {
        assert(Minit[2*bs*(2*j+1)]);
        biquad_both(TP+3*j,TPinv+3*j,&M[2*bs*(2*j+1)],P,A);
        biquad_pm1(T1+3*j,Tminus1+3*j,&M[2*bs*(2*j+1)],A);
      }
  
      poly_multiprod2(TP,gs);
      poly_multiprod2(TPinv,gs);
      poly_multiprod2_selfreciprocal(T1,gs);
      poly_multiprod2_selfreciprocal(Tminus1,gs);

      long long precompsize = poly_multieval_precomputesize(bs,2*gs+1);
      fp precomp[precompsize];
      poly_multieval_precompute(precomp,bs,2*gs+1,TI,TI+2*bs);

      fp v[bs];

      poly_multieval_postcompute(v,bs,TP,2*gs+1,TI,TI+2*bs,precomp);
      Qbatch.z = v[0];
      for (long long i = 1;i < bs;++i) fp_mul2(&Qbatch.z,&v[i]);

      poly_multieval_postcompute(v,bs,TPinv,2*gs+1,TI,TI+2*bs,precomp);
      Qbatch.x = v[0];
      for (long long i = 1;i < bs;++i) fp_mul2(&Qbatch.x,&v[i]);

      poly_multieval_postcompute(v,bs,T1,2*gs+1,TI,TI+2*bs,precomp);
      Abatch.x = v[0];
      for (long long i = 1;i < bs;++i) fp_mul2(&Abatch.x,&v[i]);

      poly_multieval_postcompute(v,bs,Tminus1,2*gs+1,TI,TI+2*bs,precomp);
      Abatch.z = v[0];
      for (long long i = 1;i < bs;++i) fp_mul2(&Abatch.z,&v[i]);

      for (long long i = 0;i < (k-1)/2-2*bs*gs;++i) {
        assert(Minit[2*i+2]);
        fp_sub3(&tmp1, &M[2*i+2].x, &M[2*i+2].z);
        fp_add3(&tmp0, &M[2*i+2].x, &M[2*i+2].z);
        fp_mul2(&Abatch.x,&tmp1);
        fp_mul2(&Abatch.z,&tmp0);
        fp_mul2(&tmp1, &Psum);
        fp_mul2(&tmp0, &Pdif);
        fp_add3(&tmp2, &tmp0, &tmp1);
        fp_mul2(&Qbatch.x, &tmp2);
        fp_sub3(&tmp2, &tmp0, &tmp1);
        fp_mul2(&Qbatch.z, &tmp2);
      }
    } else {
      for (long long i = 1;i <= (k-1)/2;++i) {
        assert(Minit[i]);
        fp_sub3(&tmp1, &M[i].x, &M[i].z);
        fp_add3(&tmp0, &M[i].x, &M[i].z);
        if (i > 1) {
          fp_mul2(&Abatch.x,&tmp1);
          fp_mul2(&Abatch.z,&tmp0);
        } else {
          Abatch.x = tmp1;
          Abatch.z = tmp0;
        }
        fp_mul2(&tmp1, &Psum);
        fp_mul2(&tmp0, &Pdif);
        fp_add3(&tmp2, &tmp0, &tmp1);
        if (i > 1) {
          fp_mul2(&Qbatch.x, &tmp2);
        } else {
          Qbatch.x = tmp2;
        }
        fp_sub3(&tmp2, &tmp0, &tmp1);
        if (i > 1) {
          fp_mul2(&Qbatch.z, &tmp2);
        } else {
          Qbatch.z = tmp2;
        }
      }
    }


    // point evaluation
    fp_sq1(&Qbatch.x);
    fp_sq1(&Qbatch.z);
    fp_mul2(&P->x, &Qbatch.x);
    fp_mul2(&P->z, &Qbatch.z);

    //compute Aed.x^k, Aed.z^k
    exp_by_squaring_(&Aed.x, &Aed.z, k);

    //compute prod.x^8, prod.z^8
    fp_sq1(&Abatch.x);
    fp_sq1(&Abatch.x);
    fp_sq1(&Abatch.x);
    fp_sq1(&Abatch.z);
    fp_sq1(&Abatch.z);
    fp_sq1(&Abatch.z);

    //compute image curve parameters
    fp_mul2(&Aed.z, &Abatch.x);
    fp_mul2(&Aed.x, &Abatch.z);

    //compute Montgomery params
    fp_add3(&A->x, &Aed.x, &Aed.z);
    fp_sub3(&A->z, &Aed.x, &Aed.z);
    fp_add2(&A->x, &A->x);
}


void xISOG_old(proj *A, proj *P, proj const *K, long long k)
{
    assert (k >= 3);
    assert (k % 2 == 1);

    fp tmp0, tmp1, tmp2, Psum, Pdif;
    proj Q, Aed, prod;

    fp_add3(&Aed.z, &A->z, &A->z);  //compute twisted Edwards curve coefficients
    fp_add3(&Aed.x, &A->x, &Aed.z);
    fp_sub3(&Aed.z, &A->x, &Aed.z);
   
    fp_add3(&Psum, &P->x, &P->z);   //precomputations
    fp_sub3(&Pdif, &P->x, &P->z);

    fp_sub3(&prod.x, &K->x, &K->z);
    fp_add3(&prod.z, &K->x, &K->z);
    
    fp_mul3(&tmp1, &prod.x, &Psum);
    fp_mul3(&tmp0, &prod.z, &Pdif);
    fp_add3(&Q.x, &tmp0, &tmp1);
    fp_sub3(&Q.z, &tmp0, &tmp1);

    proj M[k];

    M[0] = *K;
    xDBL(&M[1], A, K);
    for (long long i = 2; i < k / 2; ++i) {
        xADD(&M[i], &M[(i - 1)], K, &M[(i - 2)]);
    }

    for (long long i = 1; i < k / 2; ++i) {
        fp_sub3(&tmp1, &M[i].x, &M[i].z);
            fp_add3(&tmp0, &M[i].x, &M[i].z);
        fp_mul2(&prod.x, &tmp1);
        fp_mul2(&prod.z, &tmp0);
            fp_mul2(&tmp1, &Psum);
            fp_mul2(&tmp0, &Pdif);
            fp_add3(&tmp2, &tmp0, &tmp1);
        fp_mul2(&Q.x, &tmp2);
            fp_sub3(&tmp2, &tmp0, &tmp1);
        fp_mul2(&Q.z, &tmp2);

    }


    // point evaluation
    fp_sq1(&Q.x);
    fp_sq1(&Q.z);
    fp_mul2(&P->x, &Q.x);
    fp_mul2(&P->z, &Q.z);

    //compute Aed.x^k, Aed.z^k
    exp_by_squaring_(&Aed.x, &Aed.z, k);

    //compute prod.x^8, prod.z^8
    fp_sq1(&prod.x);
    fp_sq1(&prod.x);
    fp_sq1(&prod.x);
    fp_sq1(&prod.z);
    fp_sq1(&prod.z);
    fp_sq1(&prod.z);

    //compute image curve parameters
    fp_mul2(&Aed.z, &prod.x);
    fp_mul2(&Aed.x, &prod.z);

    //compute Montgomery params
    fp_add3(&A->x, &Aed.x, &Aed.z);
    fp_sub3(&A->z, &Aed.x, &Aed.z);
    fp_add2(&A->x, &A->x);
}
