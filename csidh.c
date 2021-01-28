#include <string.h>
#include <assert.h>

#include "csidh.h"
#include "fp.h"
#include "primes.h"
#include "rng.h"

static int useoldxISOG = 0;
static int usexISOG_2ADDSkip = 0;

void csidh_useoldxISOG(void)
{
  useoldxISOG = 1;
}

void csidh_usexISOG_2ADDSkip(void)
{
  usexISOG_2ADDSkip = 1;
}

const public_key base = {0}; /* A = 0 */

void csidh_private(private_key *priv)
{
    memset(&priv->e, 0, sizeof(priv->e));
    for (long long i = 0; i < primes_num; ) {
        int8_t buf[64];
        randombytes(buf, sizeof(buf));
        for (size_t j = 0; j < sizeof(buf); ++j) {
            if (buf[j] <= primes_max_exponent && buf[j] >= -primes_max_exponent) {
                priv->e[i / 2] |= (buf[j] & 0xf) << i % 2 * 4;
                if (++i >= primes_num)
                    break;
            }
        }
    }
}

/* compute [(p+1)/l] P for all l in our list of primes. */
/* divide and conquer is much faster than doing it naively,
 * but uses more memory. */
static void cofactor_multiples(proj *P, const proj *A, size_t lower, size_t upper)
{
    assert(lower < upper);

    if (upper - lower == 1)
        return;

    size_t mid = lower + (upper - lower + 1) / 2;

    uintbig cl = uintbig_1, cu = uintbig_1;
    for (size_t i = lower; i < mid; ++i)
        uintbig_mul3_64(&cu, &cu, primes[i]);
    for (size_t i = mid; i < upper; ++i)
        uintbig_mul3_64(&cl, &cl, primes[i]);

    xMUL(&P[mid], A, &P[lower], &cu);
    xMUL(&P[lower], A, &P[lower], &cl);

    cofactor_multiples(P, A, lower, mid);
    cofactor_multiples(P, A, mid, upper);
}

/* never accepts invalid keys. */
bool validate(public_key const *in)
{
    const proj A = {in->A, fp_1};

    do {

        proj P[primes_num];
        fp_random(&P->x);
        P->z = fp_1;

        /* maximal 2-power in p+1 */
        xDBL(P, &A, P);
        xDBL(P, &A, P);

        cofactor_multiples(P, &A, 0, primes_num);

        uintbig order = uintbig_1;

        for (long long i = primes_num - 1; i < primes_num; --i) {

            /* we only gain information if [(p+1)/l] P is non-zero */
            if (memcmp(&P[i].z, &fp_0, sizeof(fp))) {

                uintbig tmp;
                uintbig_set(&tmp, primes[i]);
                xMUL(&P[i], &A, &P[i], &tmp);

                if (memcmp(&P[i].z, &fp_0, sizeof(fp)))
                    /* P does not have order dividing p+1. */
                    return false;

                uintbig_mul3_64(&order, &order, primes[i]);

                if (uintbig_sub3(&tmp, &fp_four_sqrt_p, &order)) /* returns borrow */
                    /* order > 4 sqrt(p), hence definitely supersingular */
                    return true;
            }
        }

    /* P didn't have big enough order to prove supersingularity. */
    } while (1);
}

/* compute x^3 + Ax^2 + x */
static void montgomery_rhs(fp *rhs, fp const *A, fp const *x)
{
    fp tmp;
    *rhs = *x;
    fp_sq1(rhs);
    fp_mul3(&tmp, A, x);
    fp_add2(rhs, &tmp);
    fp_add2(rhs, &fp_1);
    fp_mul2(rhs, x);
}

/* totally not constant-time. */
void action(public_key *out, public_key const *in, private_key const *priv)
{
    uintbig k[2];
    uintbig_set(&k[0], 4); /* maximal 2-power in p+1 */
    uintbig_set(&k[1], 4); /* maximal 2-power in p+1 */

    uint8_t e[2][primes_num];

    for (long long i = 0; i < primes_num; ++i) {

        int8_t t = (int8_t) (priv->e[i / 2] << i % 2 * 4) >> 4;

        if (t > 0) {
            e[0][i] = t;
            e[1][i] = 0;
            uintbig_mul3_64(&k[1], &k[1], primes[i]);
        }
        else if (t < 0) {
            e[1][i] = -t;
            e[0][i] = 0;
            uintbig_mul3_64(&k[0], &k[0], primes[i]);
        }
        else {
            e[0][i] = 0;
            e[1][i] = 0;
            uintbig_mul3_64(&k[0], &k[0], primes[i]);
            uintbig_mul3_64(&k[1], &k[1], primes[i]);
        }
    }

    proj A = {in->A, fp_1};

    bool done[2] = {false, false};

    do {

        assert(!memcmp(&A.z, &fp_1, sizeof(fp)));

        proj P;
        fp_random(&P.x);
        P.z = fp_1;

        fp rhs;
        montgomery_rhs(&rhs, &A.x, &P.x);
        bool sign = !fp_issquare(&rhs);

        if (done[sign])
            continue;

        xMUL(&P, &A, &P, &k[sign]);

        done[sign] = true;

        for (long long i = primes_num-1; i >= 0; --i) {  //changed loop direction

            if (e[sign][i]) {

                uintbig cof = uintbig_1;
                for (long long j = i - 1; j >= 0; --j)   //changed loop direction
                    if (e[sign][j])
                        uintbig_mul3_64(&cof, &cof, primes[j]);

                proj K;
                xMUL(&K, &A, &P, &cof);

                if (memcmp(&K.z, &fp_0, sizeof(fp))) {

                    if (usexISOG_2ADDSkip)
                      xISOG_2ADDSkip(&A, &P, &K, primes[i], nfor2addskip[i]); 
                    else if (useoldxISOG)
                      xISOG_old(&A, &P, &K, primes[i]);
		            else 
                      xISOG(&A, &P, &K, primes[i]);

                    if (!--e[sign][i])
                        uintbig_mul3_64(&k[sign], &k[sign], primes[i]);

                }

            }

            done[sign] &= !e[sign][i];
        }

        fp_inv(&A.z);
        fp_mul2(&A.x, &A.z);
        A.z = fp_1;

    } while (!(done[0] && done[1]));

    out->A = A.x;
}

/* includes public-key validation. */
bool csidh(public_key *out, public_key const *in, private_key const *priv)
{
    if (!validate(in)) {
        fp_random(&out->A);
        return false;
    }
    action(out, in, priv);
    return true;
}

