SCC=gcc
CC=clang -O3 -Os -march=native -mtune=native -std=gnu99 -pedantic -Wall -Wextra

all: test512.out oldtest512 \
benchmults512.out benchcycles512.out oldbench512 \
ubench512 umults512 \
test1024.out oldtest1024 \
benchmults1024.out benchcycles1024.out oldbench1024 \
ubench1024 umults1024

test512: test.c steps_default.o \
uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o Makefile
	$(CC) -DBITS=512 -o test512 test.c steps_default.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o

test1024: test.c steps_default.o \
uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o Makefile
	$(CC) -DBITS=1024 -o test1024 test.c steps_default.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o

test512.out: test512 test512.exp
	# 20 seconds on 3GHz Skylake:
	time ./test512 > test512.out
	cmp test512.out test512.exp

test1024.out: test1024 test1024.exp
	# 4 minutes on 3GHz Skylake:
	time ./test1024 > test1024.out
	cmp test1024.out test1024.exp

oldtest512: oldtest.c steps_default.o \
uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o Makefile
	$(CC) -DBITS=512 -o oldtest512 oldtest.c steps_default.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o

oldtest1024: oldtest.c steps_default.o \
uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o Makefile
	$(CC) -DBITS=1024 -o oldtest1024 oldtest.c steps_default.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o

tunemults512: tunemults.c steps_default.o \
uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o Makefile
	$(CC) -DBITS=512 -o tunemults512 tunemults.c steps_default.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o

tunemults1024: tunemults.c steps_default.o \
uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o Makefile
	$(CC) -DBITS=1024 -o tunemults1024 tunemults.c steps_default.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o

tunemults512.out: tunemults512 Makefile
	# 1 second on 3GHz Skylake:
	time ./tunemults512 > tunemults512.out

tunemults1024.out: tunemults1024 Makefile
	# 30 seconds on 3GHz Skylake:
	time ./tunemults1024 > tunemults1024.out

steps_tunemults512.c: tunemults512.out tune2c Makefile
	./tune2c < tunemults512.out > steps_tunemults512.c

steps_tunemults1024.c: tunemults1024.out tune2c Makefile
	./tune2c < tunemults1024.out > steps_tunemults1024.c

steps_tunemults512.o: steps_tunemults512.c steps.h Makefile
	$(CC) -DBITS=512 -c steps_tunemults512.c

steps_tunemults1024.o: steps_tunemults1024.c steps.h Makefile
	$(CC) -DBITS=1024 -c steps_tunemults1024.c

benchmults512: benchmults.c steps_tunemults512.o \
uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o Makefile
	$(CC) -DBITS=512 -o benchmults512 benchmults.c steps_tunemults512.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o

benchmults1024: benchmults.c steps_tunemults1024.o \
uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o Makefile
	$(CC) -DBITS=1024 -o benchmults1024 benchmults.c steps_tunemults1024.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o

benchmults512.out: benchmults512
	# 40 seconds on 3GHz Skylake:
	time ./benchmults512 > benchmults512.out

benchmults1024.out: benchmults1024
	# 4.2 minutes on 3GHz Skylake:
	time ./benchmults1024 > benchmults1024.out

tunecycles512: tunecycles.c steps_default.o \
uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o Makefile
	$(CC) -DBITS=512 -o tunecycles512 tunecycles.c steps_default.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o

tunecycles1024: tunecycles.c steps_default.o \
uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o Makefile
	$(CC) -DBITS=1024 -o tunecycles1024 tunecycles.c steps_default.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o

tunecycles512.out: tunecycles512 Makefile
	# 35 seconds on 3GHz Skylake:
	time ./tunecycles512 > tunecycles512.out

tunecycles1024.out: tunecycles1024 Makefile
	# 17 minutes on 3GHz Skylake:
	time ./tunecycles1024 > tunecycles1024.out

steps_tunecycles512.c: tunecycles512.out tune2c Makefile
	./tune2c < tunecycles512.out > steps_tunecycles512.c

steps_tunecycles1024.c: tunecycles1024.out tune2c Makefile
	./tune2c < tunecycles1024.out > steps_tunecycles1024.c

steps_tunecycles512.o: steps_tunecycles512.c steps.h Makefile
	$(CC) -c steps_tunecycles512.c

steps_tunecycles1024.o: steps_tunecycles1024.c steps.h Makefile
	$(CC) -c steps_tunecycles1024.c

benchcycles512: benchcycles.c steps_tunecycles512.o \
uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o Makefile
	$(CC) -DBITS=512 -o benchcycles512 benchcycles.c steps_tunecycles512.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o

benchcycles1024: benchcycles.c steps_tunecycles1024.o \
uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o Makefile
	$(CC) -DBITS=1024 -o benchcycles1024 benchcycles.c steps_tunecycles512.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o

benchcycles512.out: benchcycles512
	# 1.4 minutes on 3GHz Skylake:
	time ./benchcycles512 > benchcycles512.out

benchcycles1024.out: benchcycles1024
	# 10 minutes on 3GHz Skylake:
	time ./benchcycles1024 > benchcycles1024.out

umults512: umults.c steps_default.o \
uint512.o fp512.o poly512.o rng.o steps.o primes512.o mont512.o Makefile
	$(CC) -DBITS=512 -o umults512 umults.c steps_default.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o

umults1024: umults.c steps_default.o \
uint1024.o fp1024.o poly1024.o rng.o steps.o primes1024.o mont1024.o Makefile
	$(CC) -DBITS=1024 -o umults1024 umults.c steps_default.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o

ubench512: ubench.c steps_default.o \
uint512.o fp512.o poly512.o rng.o steps_default.o steps.o primes512.o mont512.o Makefile
	$(CC) -DBITS=512 -o ubench512 ubench.c steps_default.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o

ubench1024: ubench.c steps_default.o \
uint1024.o fp1024.o poly1024.o rng.o steps_default.o steps.o primes1024.o mont1024.o Makefile
	$(CC) -DBITS=1024 -o ubench1024 ubench.c steps_default.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o

oldbench512: oldbench.c steps_default.o \
uint512.o fp512.o poly512.o rng.o steps.o primes512.o mont512.o csidh512.o Makefile
	$(CC) -DBITS=512 -o oldbench512 oldbench.c steps_default.o \
		uint512.o rng.o fp512.o poly512.o steps.o primes512.o mont512.o csidh512.o

oldbench1024: oldbench.c steps_default.o \
uint1024.o fp1024.o poly1024.o rng.o steps.o primes1024.o mont1024.o csidh1024.o Makefile
	$(CC) -DBITS=1024 -o oldbench1024 oldbench.c steps_default.o \
		uint1024.o rng.o fp1024.o poly1024.o steps.o primes1024.o mont1024.o csidh1024.o

csidh512.o: csidh.c csidh.h Makefile
	$(CC) -DBITS=512 -o csidh512.o -c csidh.c

csidh1024.o: csidh.c csidh.h Makefile
	$(CC) -DBITS=1024 -o csidh1024.o -c csidh.c

mont512.o: mont.c mont.h uintbig.h fp.h steps.h Makefile
	$(CC) -DBITS=512 -o mont512.o -c mont.c

mont1024.o: mont.c mont.h uintbig.h fp.h steps.h Makefile
	$(CC) -DBITS=1024 -o mont1024.o -c mont.c

poly512.o: poly.c poly.h fp.h Makefile
	$(CC) -DBITS=512 -o poly512.o -c poly.c

poly1024.o: poly.c poly.h fp.h Makefile
	$(CC) -DBITS=1024 -o poly1024.o -c poly.c

primes512.o: primes512.c primes.h Makefile
	$(CC) -DBITS=512 -c primes512.c

primes1024.o: primes1024.c primes.h Makefile
	$(CC) -DBITS=1024 -c primes1024.c

fp512.o: fp512.s fp.h Makefile
	$(SCC) -c fp512.s

fp1024.o: fp1024.s fp.h Makefile
	$(SCC) -c fp1024.s

uint512.o: uint512.s uintbig.h Makefile
	$(SCC) -c uint512.s

uint1024.o: uint1024.s uintbig.h Makefile
	$(SCC) -c uint1024.s

steps.o: steps.c steps.h Makefile
	$(CC) -c steps.c

steps_default.o: steps_default.c steps.h Makefile
	$(CC) -c steps_default.c

rng.o: rng.c rng.h Makefile
	$(CC) -c rng.c
