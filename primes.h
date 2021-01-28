#ifndef primes_h
#define primes_h

#if BITS == 512
#define primes_num 74
#define primes_max_exponent 5
#elif BITS == 1024
#define primes_num 130
#define primes_max_exponent 2
#else
#error BITS must be 512 or 1024
#endif

extern const long long primes[];
extern const long long nfor2addskip[];

#endif
