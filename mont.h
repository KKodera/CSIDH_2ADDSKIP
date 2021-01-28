#ifndef MONT_H
#define MONT_H

#include "uintbig.h"
#include "fp.h"

/* P^1 over fp. */
typedef struct proj {
    fp x;
    fp z;
} proj;

void xDBL(proj *Q, proj const *A, proj const *P);
void xADD(proj *S, proj const *P, proj const *Q, proj const *PQ);
void xDBLADD(proj *R, proj *S, proj const *P, proj const *Q, proj const *PQ, proj const *A);
void xMUL(proj *Q, proj const *A, proj const *P, uintbig const *k);
void xISOG(proj *A, proj *P, proj const *K, long long k);
void xISOG_old(proj *A, proj *P, proj const *K, long long k);
void xISOG_2ADDSkip(proj *A, proj *P, proj const *K, long long k, long long n);

#endif
