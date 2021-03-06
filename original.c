/**************************************************************
 * Warren D Smith's IEVS program
 * 
 * Needs at least 4m for the stack. This requires link args:
 *      gcc "-Wl,--stack,4194304" .\IEVS.c 
 */
#define TRUE (0 < 1)
#define FALSE (1 < 0)
#define MSWINDOWS FALSE
#if MSWINDOWS
#include "stdafx.h" /*needed if Microsoft Windows OS, unwanted if LINUX; will be more of that*/
#endif
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#if MSWINDOWS
#include <time.h>
#include <sys/timeb.h>
#include <process.h>
#else
#include <sys/time.h>
#include <limits.h>
#include <unistd.h>
#endif

/* #define NDEBUG    uncomment this line if want to turn off asserts for speed */
#define VERSION 3.24
#define VERSIONYEAR 2007
#define VERSIONMONTH 2

/**IEVS ***** Warren D. Smiths's 2007 infinitely extendible voting system comparison engine ****
Copyright (C): Warren D. Smith 2007.  Anybody can use this non-commercially 
provided they acknowledge me, notify me (and make available for my unrestricted use)
re any & all code improvements created or bugs found.  If you want commercial use,
then we'll negotiate.  

On Unix, Mac OSX, and Linux: Compilation:  gcc IEVS.c -Wall -lm -O6 -o IEVS
    for extra speed (but less safety) add the #define NDEBUG line at top.
On MSWINDOWS:   change #define MSWINDOWS to TRUE.  Should work, see notes below by Cary -
    thanks much to David Cary for the MSWINDOWS port!!  (Except that he introduced a bunch
    of bugs with careless typecasts, which hopefully I now have fixed...)
What election methods are available?:    fgrep EMETH IEVS.c
What utility-generators now available?:  fgrep UTGEN IEVS.c

This program should also work (with minor modifications) under microsoft windows OS.
David Cary successfully ported it and his notes on how he did it, are below.
As a side effect, Cary also noticed how to speed up Yee-picture-generation significantly
and fixed a bug in OutputCompressedBarray(). :)
If you do such porting, please report your experience, how you compiled it, 
etc.   warren.wds AT gmail.com.

Software Architecture:
I. voting methods.
II. voting strategies.
III. ignorance generators
IV. utility generators

Anybody can add new voting methods, new strategies, or new utility generators.
The idea is to build a "Chinese menu" system which can investigate A*B*C*D kinds of scenarios
BUT the effort to write it is only A+B+C+D.

The information-flow-direction is  IV-->III-->II-->I-->winners-->regrets.

I am initially writing this with not very many of each.  It is a "skeleton"
system which can be fleshed out later by adding more of each of I,II,III,IV.
Please send your contributed routines to warren.wds AT gmail.com .
You should be able to make an implementation of a new method pretty easily by
imitating some already-implemented methods similar to yours.
BUGGY: Rouse??, Copeland??
As-yet unimplemented voting methods include:
ICA, Maxtree, range+runoff(based on range ballots with ties discarded),
RRV+runoff, Sarvo-methods, Banks set, Boehm, Dodgson.
Asset & Candidate-withdrawal-option-IRV (both don't really fit into our model).
To add a new voting method you need to:
add your new EMETH subroutine, and you need to change 3 more lines:
  #define NumMethods PutCorrect#Methodshere
  and the case lines in GimmeWinner() and PrintMethName().
(If you add a new "core" method also must change NumCoreMethods and 
perhaps renumber the methods...)

Currently only rating-based, strict-ranking-based, and approval-based methods (& combinations)
are supported - equalities are not allowed in rankings.  ("3-slot" methods
also supported.)  To extend the code to allow
equalities, I suggest adding a boolean vector
with v[x]=TRUE meaning x is really tied with, not below,
the candidate immediately preceding.  That is a later goal.

HonestyStrat() now supports an arbitrary mixture of honest and (one kind of) strategic voters.  
More strategies can/will be added later.

3 utility generators now implemented (some parameterized), plus 
another "reality-based" utility generator based on
Tideman-et-al's real-world election collection.

Future plans/ideas:  translate from C into D?
(Templates & automatic array bounds checking would really help.)
auto-searcher for property violations?  
L1 utils should be skewed distn.
Allow a user-selected voting-method subset.
Targeted BR finder (new user interface to BR finder).
Other strat generators.
Use of 2-candidate elections is knd of silly.
Need to make better controls on summarizers so you can ask it to summarize only
SUBSETS of the data (such as, honest voters only, large #s of candidates only, etc)...
More realistic strategies for plur+runoff, MCA, IRNR, Benham2AppRunoff?
Build good hybrid voting methods?
Add feature to input votes from user and tally election.
Two-humped Gaussian distributions?
RandomWinner regret mean & variance should be computed more exactly.
DMC & the like - do the approvals and rankings correspond with strategy? Should they?
Summarizer: Also look at dominance relations, "approval".
***************************

Now notes by David Cary, 2007-02-14 re modifications to port IEVS 2.58 from LINUX to 
Windows XP using Microsoft Visual Studio .Net 2003.  

David Cary's Changes (not listing ones WDS did anyhow) include:
 1) Fix compile errors due to passing an int[] to a function that expects a uint[],
 2) Fix compile errors due to passing a uint[] to a function that expects an int[],
 3) Fix runtime error caused by opening the .bmp output file in text mode, to avoid
      LF bytes being written as CR LF combinations.
 4) Add an include for "stdafx.h".

 This program was successfully compiled and run as a C++ program after making the following
 adjustments to the default Visual Studio Project:
 1)  Increase the stack size to 4 MB (under Linker/System)
        (the default is 1 MB, but sizeof(edata) is about 2.2 MB and lives on the stack).

 All reported warnings were eliminated by:
 1) Updating the Visual Studio Project to not report 64-bit issues
      (under "C/C++" / General)
 2) Demoting warning messages #4018 & 4244 to warning level 4 by adding command line options
      /w44018 /w44244 under "C/C++" / Command Line.
      These messages warn about implicit int/uint conversions and conversions from __int64
      to unsigned int.
****************************/

#define until(x) while (!(x))
#define uint unsigned int
#define bool char
#define uint32 unsigned int
#define uint64 unsigned long long
#define uchar unsigned char
#define schar signed char
#define real double

#define HUGE HUGE_VAL /* Adding bridge macro to math.h macro WGA */

#define PI 3.14159265358979323844

#define MaxNumCands 32
#define MaxNumVoters 2048
#define MaxNumIssues 8
#define MaxScenarios 17424
#define NumMethods 68 /*EMETH the # in this line needs to change if add new voting method*/
#define NumSlowMethods 8
#define NumFastMethods (NumMethods - NumSlowMethods)
#define NumUtilGens 11 /*UTGEN the # in this line needs to change if add new util gen*/
#define NumCoreMethods 12
#define CWSPEEDUP FALSE /* TRUE causes it to be faster, but FALSE better for diagnosing bugs */

#define HTMLMODE 1         /*output adding "HTML table" formatting commands*/
#define TEXMODE 2          /*output adding "TeX table" formatting commands*/
#define NORMALIZEREGRETS 4 /*Output "normalized" Regrets so RandomWinner=1, SociallyBest=0. */
#define SORTMODE 8         /*Output with voting methods in sorted order by increasing regrets.*/
#define SHENTRUPVSR 16     /*Output "Shentrup normalized" Regrets so RandomWinner=0%, SociallyBest=100%. */
#define OMITERRORBARS 32
#define VBCONDMODE 64 /*vote-based condorcet winner agreement counts; versus true utility based CWs*/
#define DOAGREETABLES 128
#define ALLMETHS 256
#define TOP10METHS 512
uint BROutputMode = 0;

/******************** GENERAL PURPOSE routines having nothing to do with voting: ******/

int EulerPrimePoly(int n) { return (n * n + n + 41); } /*prime for n=0..39*/
const int Pow2Primes[] = {2, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381};
/** Greatest prime <=2^n. **/

/******** Fns to deal with sets represented by bit-vectors in 1 machine word: ******/
bool SingletonSet(uint x) { return ((x & (x - 1)) == 0); } /*assumes non-empty*/
bool StrictSuperset(uint x, uint y) { return ((x & y) == y && x != y); }
bool EmptySet(uint x) { return (x == 0); }
int CardinalitySet(uint x)
{
    int ct = 0;
    while (x)
    {
        ct++;
        x &= x - 1;
    }
    return (ct);
}

/****** convenient fns: *******/
real SquareReal(real x) { return x * x; }
int SquareInt(int x) { return x * x; }
int AbsInt(int x)
{
    if (x < 0)
        return -x;
    else
        return x;
}
real PosReal(real x)
{
    if (x > 0.0)
        return x;
    else
        return 0.0;
}
uint PosInt(int x)
{
    if (x > 0)
        return x;
    else
        return 0;
}
int SignInt(int x)
{
    if (x > 0)
        return 1;
    else if (x < 0)
        return -1;
    else
        return 0;
}
int SignReal(real x)
{
    if (x > 0)
        return 1;
    else if (x < 0)
        return -1;
    else
        return 0;
}
int MaxInt(int a, int b) { return (((a) > (b)) ? (a) : (b)); }
real MaxReal(real a, real b) { return (((a) > (b)) ? (a) : (b)); }
int MinInt(int a, int b) { return (((a) < (b)) ? (a) : (b)); }
real MinReal(real a, real b) { return (((a) < (b)) ? (a) : (b)); }

uint GCD(uint a, uint b)
{ /*Euclid alg*/
    if (a > b)
    {
        a %= b;
        if (a == 0)
            return b;
    }
    for (;;)
    {
        b %= a;
        if (b == 0)
            return a;
        a %= b;
        if (a == 0)
            return b;
    }
}

uint LCMfact[32];

void BuildLCMfact()
{
    uint j, x;
    printf("\nComputing sequence LCM(1,2,...N) for N=1..22:\n");
    LCMfact[0] = 1;
    for (j = 1; j < 23; j++)
    { /*LCMfact[23]=5354228880 won't fit in 32 bit word*/
        LCMfact[j] = LCMfact[j - 1];
        x = LCMfact[j] % j;
        if (x)
            LCMfact[j] *= j / GCD(x, j);
    }
    printf("LCMfact[%d]=%u\n", j, LCMfact[j]);
}

/***************************************** other "Artin primes" are 
3, 5, 11, 13, 19, 29, 37, 53, 59, 61, 67, 83, 101, 107, 131, 139, 149, 163, 173, 179, 181, 
197, 211, 227, 269, 293, 317, 347, 349, 373, 379, 389, 419, 421, 443, 461, 467, 491, 509, 
523, 541, 547, 557, 563, 587, 613, 619, 653, 659, 661, 677, 701, 709, 757, 773, 787, 797
i.e. these are primes which have 2 as a primitive root.
This routine finds the greatest Artin prime P with P<=x.  (Not intended to be fast.)
  *******************************************/
uint ARTINPRIME;
uint FindArtinPrime(uint x)
{
    uint j, p, k;
    p = x;
    if (p % 2 == 0)
        p--; /* make p be odd */
    for (; p > 2; p -= 2)
    {
        for (j = 4, k = 3; j != 2; j = (j * 2) % p, k++)
        {
            if (k >= p)
            {
                return (p);
            }
        }
    }
    printf("FindArtinPrime failed - terminating\n");
    exit(EXIT_FAILURE);
}

void PrintNSpaces(int N)
{
    int i;
    for (i = 0; i < N; i++)
        putchar(' ');
}
/****** convenient constants: *******/
#define BIGINT 0x7FFFFFFF
#define MAXUINT UINT_MAX
//#define MAXUINT ((uint)((255<<48)|(255<<40)|(255<<32)|(255<<24)|(255<<16)|(255<<8)|(255)))
/* defn works on 8,16,32, and 64-bit machines */

uint32 BLC32x[60]; /* 32*60=1920 bits of state. Must be nonzero mod P. */
int BLC32NumLeft;

/********************************************************
Warren D. Smith 2001
**********************************************************
Linear congruential pseudo-random number generator mod P,
where P is the enormous prime (578 base-10 digits long; 
60 words long if each word is 32 bits)
  P = [2^(48*32) - 1] * 2^(12*32) + 1.
This prime can yield PRNGs suitable for use on 
computers with w-bit words, w=8,16,32,48,64,96,128.
The following code is intended for w=32.
The fact that 2^(60*32) = 2^(12*32) - 1 (mod P)
makes modular arithmetic mod P easy, and is the
reason this particular P was chosen.
The period of our generator is P-1.
***********************************************************
Although it is usually easy to detect the nonrandomness of
linear congruential generators because they generate d-tuples
lying on a lattice, in the present case the "grain size" of the
lattice is invisibly small (far less than a least significant bit),
for 32-bit words, if 1<=d<=180. If you want 64-bit words, then need
to concatenate two 32-bit words, and then grain size invisibly small
if 1<=d<=90. These bounds are conservative; in fact I suspect
one is ok, even for 64-bit words, even in up to 1000 dimensions.
***********************************************************
Some even more enormous primes which we have not used are:
[2^{59*32} - 1] * 2^{8 *32} + 1,
[2^{63*32} - 1] * 2^{24*32} + 1,
[2^{69*32} - 1] * 2^{14*32} + 1,
[2^{95*32} - 1] * 2^{67*32} + 1,
[2^{99*32} - 1] * 2^{35*32} + 1;
these would also be suitable for (8,16,32)-bit computers,
and the second of them would also be good for (48,96)-bit computers.
Unfortunately the factorization of P-1 is not known for the last 3 
I've listed here, preventing you from being certain you've found a
primitive root mod that P. A still more enormous prime is
  [2^4253 - 1] * 2^4580 + 1    [2659 digits long!]
(where note 2^4253-1 is also prime so that factorizing P-1 is
trivial) but doing arithmetic mod this P is (although still fairly
easy) less pleasant because bit-shifting is required.
*************************************************************/
uint32 BigLinCong32()
{
    uint32 y[120];
    int i;
    uint64 u;

    if (BLC32NumLeft == 0)
    {
        /* Need to refill BLC32x[0..59] with 60 new random numbers: */

        /****************************************************************
 * If BLC32x[0..59] is the digits, LS..MS, of a number in base 2^w,
 * then the following code fragment puts A times that number 
 * in y[0..119].  Here
 *  A = 1284507170 * 2^(w*3) + 847441413 * 2^(w*44) + 650134147 * 2^(w*59)
 * is a "good" primitive root mod P, if w=32.
 *****************************************************************/
#define lohalf(x) (uint32)(x)
#define A1 (uint64)1284507170
#define A2 (uint64)847441413
#define A3 (uint64)650134147
        for (i = 0; i < 3; i++)
        {
            y[i] = 0;
        }
        u = 0;
        for (/*i=3*/; i < 44; i++)
        {
            u += A1 * BLC32x[i - 3];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        for (/*i=44*/; i < 59; i++)
        {
            u += A1 * BLC32x[i - 3];
            u += A2 * BLC32x[i - 44];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        for (/*i=59*/; i < 60 + 3; i++)
        {
            u += A1 * BLC32x[i - 3];
            u += A2 * BLC32x[i - 44];
            u += A3 * BLC32x[i - 59];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        for (/*i=60+3*/; i < 60 + 44; i++)
        {
            u += A2 * BLC32x[i - 44];
            u += A3 * BLC32x[i - 59];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        for (/*i=60+44*/; i < 60 + 59; i++)
        {
            u += A3 * BLC32x[i - 59];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        /*i=60+59=119*/
        y[i] = lohalf(u);
#undef A1
#undef A2
#undef A3
        /*************************************************************
 * If y[0..119] is the digits, LS..MS, of a number in base 2^w,
 * then the following code fragment replaces that number with
 * its remainder mod P in y[0..59]  (conceivably the result will
 * be >P, but this does not matter; it will never be >=2^(w*60)).
 **************************************************************/
        u = 1; /*borrow*/
#define AllF 0xffffffff
        /* Step 1: y[0..72] = y[0..59] + y[60..119]shift12 - y[60..119]: */
        for (i = 0; i < 12; i++)
        {
            u += y[i];
            u += (uint64)~y[60 + i];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        for (/*i=12*/; i < 60; i++)
        {
            u += y[i];
            u += y[48 + i];
            u += (uint64)~y[60 + i];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        for (/*i=60*/; i < 72; i++)
        {
            u += AllF;
            u += y[48 + i];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        assert(u > 0);
        y[72] = (uint32)(u - 1); /*unborrow*/

        /*  Step 2: y[0..60] = y[0..59] + y[60..72]shift12  - y[60..72]: */
        u = 1; /*borrow*/
        for (i = 0; i < 12; i++)
        {
            u += y[i];
            u += (uint64)~y[60 + i];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        /*i=12*/
        u += y[i] + y[48 + i];
        u += (uint64)~y[60 + i];
        y[i] = lohalf(u);
        u = u >> 32;
        i++;
        for (/*i=13*/; i < 25; i++)
        {
            u += AllF;
            u += y[i];
            u += y[48 + i];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        for (/*i=25*/; i < 60; i++)
        {
            u += AllF;
            u += y[i];
            y[i] = lohalf(u);
            u = u >> 32;
        }
        /*i=60*/
        assert(u > 0);
        y[i] = (uint32)(u - 1); /*unborrow*/

        /*It is rare that any iterations of this loop are needed:*/
        while (y[60] != 0)
        {
            printf("rare loop\n");
            /*Step 3+:  y[0..60] = y[0..59] + y[60]shift12 - y[60]:*/
            u = 1; /*borrow*/
            u += y[0];
            u += (uint64)~y[60];
            y[0] = lohalf(u);
            u = u >> 32;
            for (i = 1; i < 12; i++)
            {
                u += AllF;
                u += y[i];
                y[i] = lohalf(u);
                u = u >> 32;
            }
            /*i=12*/
            u += AllF;
            u += y[i];
            u += y[60];
            y[i] = lohalf(u);
            u = u >> 32;
            i++;
            for (/*i=13*/; i < 60; i++)
            {
                u += AllF;
                u += y[i];
                y[i] = lohalf(u);
                u = u >> 32;
            }
            /*i=60*/
            assert(u > 0);
            y[i] = (uint32)(u - 1); /*unborrow*/
        }
#undef AllF
#undef lohalf

        /* Copy y[0..59] into BLC32x[0..59]: */
        for (i = 0; i < 60; i++)
        {
            BLC32x[i] = y[i];
        }
        /*printf("%u\n", BLC32x[44]);*/
        BLC32NumLeft = 60;
    }
    /* (Else) We have random numbers left, so return one: */
    BLC32NumLeft--;
    return BLC32x[BLC32NumLeft];
}

void testbiglincong()
{
    int i;
    /* lexically minimal permissible seed: */
    for (i = 0; i < 60; i++)
    {
        BLC32x[i] = 0;
    }
    BLC32x[0] = 1;
    BLC32NumLeft = 0;

    for (i = 0; i < 599; i++)
    {
        BigLinCong32();
    }
    printf("%u %u %u %u %u\n",
           BigLinCong32(), BigLinCong32(),
           BigLinCong32(), BigLinCong32(), BigLinCong32());

    for (i = 0; i < 12; i++)
    {
        printf("%8x %8x %8x %8x %8x\n",
               BigLinCong32(), BigLinCong32(),
               BigLinCong32(), BigLinCong32(), BigLinCong32());
    }
}

/********************************
MAPLE script to check it works:

w := 32;
A := 1284507170 * 2^(w*3) + 847441413 * 2^(w*44) + 650134147 * 2^(w*59);
P := (2^(48*32) - 1) * 2^(12*32) + 1;
for i from 1 to 11 do
   print( floor((A &^ i mod P) / 2^(44*w)) mod (2^w) );
od;
ap := A &^ 11 mod P;
ap mod (2^w);
quit;
********************
MAPLE output:
847441413
4038410930
102374915
470100141
3896743552
243412576
1911259311
1640083353
4014446395
2679952782
4087228849
and
2475219032

C output:
847441413
4038410930
102374915
470100141
3896743552
243412576
oops
2990118053
2614294516
3539391572
1589778147
1758817216
2847725135 1364008005 3563722108 2020382641 1091616930
*************************/

uint32 RandUint()
{ /* returns random uint32 */
    return BigLinCong32();
}

real Rand01()
{ /* returns random uniform in [0,1] */
    return ((BigLinCong32() + 0.5) / (1.0 + MAXUINT) + BigLinCong32()) / (1.0 + MAXUINT);
}

void InitRand(uint seed)
{ /* initializes the randgen */
    int i;
    int seed_sec = 0, processId = 0;
    uint seed_usec = 0;
#if MSWINDOWS
    tm *locTime;
    _timeb currTime;
    time_t now;
#else
    struct timeval tv;
#endif
    if (seed == 0)
    {
        printf("using time of day and PID to generate seed");
#if MSWINDOWS
        now = time(NULL);
        locTime = localtime(&now);
        _ftime(&currTime);
        seed_usec = currTime.millitm * 1001;
        seed_sec = locTime->tm_sec + 60 * (locTime->tm_min + 60 * locTime->tm_hour);
        processId = _getpid();
#else
        gettimeofday(&tv, 0);
        seed_sec = tv.tv_sec;
        seed_usec = tv.tv_usec;
        processId = getpid();
#endif
        seed = 1000000 * (uint)seed_sec + (uint)seed_usec +
               (((uint)processId) << 20) + (((uint)processId) << 10);
        printf("=%u\n", seed);
    }
    for (i = 0; i < 60; i++)
    {
        BLC32x[i] = 0;
    }
    BLC32x[0] = seed;
    BLC32NumLeft = 0;
    for (i = 0; i < 599; i++)
    {
        BigLinCong32();
    }
    printf("Random generator initialized with seed=%u:\n", seed);
    for (i = 0; i < 7; i++)
    {
        printf("%.6f ", Rand01());
    }
    printf("\n");
}

void TestRand01()
{
    int i, y, ct[10];
    real x, s, mx, mn, v;
    s = 0.0;
    v = 0.0;
    mn = HUGE;
    mx = -HUGE;
    for (i = 0; i < 10; i++)
        ct[i] = 0;
    printf("Performing 100000 randgen calls to test that randgen[0,1] behaving ok:\n");
    for (i = 0; i < 100000; i++)
    {
        x = Rand01();
        s += x;
        if (mx < x)
            mx = x;
        if (x < mn)
            mn = x;
        v += SquareReal(x - 0.5);
        y = (int)(x * 10.0);
        if (x >= 0 && y < 10)
            ct[y]++;
    }
    printf("mean=%g(should be 1/2)  min=%f  max=%g   variance=%g(should be 1/12=%g)\n",
           s / 100000.0, mn, mx, v / 100000.0, 1 / 12.0);
    printf("counts in 10 bins 0-0.1, 0.1-0.2, etc: ");
    for (i = 0; i < 10; i++)
        printf(" %d", ct[i]);
    printf("\n");
    fflush(stdout);
}

bool RandBool()
{ /* returns random boolean */
    if (Rand01() > 0.5)
        return TRUE;
    return FALSE;
}

real RandExpl()
{ /* returns standard exponential (density=exp(-x) for x>0) random deviate */
    real x;
    do
    {
        x = Rand01();
    } while (x == 0.0);
    return -log(x);
}

void TestRandExpl()
{
    int i, y, ct[10];
    real x, s, mx, mn, v;
    s = 0.0;
    v = 0.0;
    mn = HUGE;
    mx = -HUGE;
    for (i = 0; i < 10; i++)
        ct[i] = 0;
    printf("Performing 100000 randgen calls to test that expl randgen behaving ok:\n");
    for (i = 0; i < 100000; i++)
    {
        x = RandExpl();
        s += x;
        if (mx < x)
            mx = x;
        if (x < mn)
            mn = x;
        v += x * x;
        y = (int)(x * 10);
        if (x >= 0 && y < 10)
            ct[y]++;
    }
    printf("mean=%g(should be 1)  min=%f  max=%g   variance=%g(should be 2?)\n",
           s / 100000.0, mn, mx, v / 100000.0);
    printf("counts in 10 bins 0-0.1, 0.1-0.2, etc: ");
    for (i = 0; i < 10; i++)
        printf(" %d", ct[i]);
    printf("\n");
    fflush(stdout);
}

real RandNormal()
{ /* returns standard Normal (gaussian variance=1 mean=0) deviate */
    real w, x1;
    static real x2;
    static bool ready = FALSE;
    if (ready)
    {
        ready = FALSE;
        return x2;
    }
    do
    {
        x1 = 2 * Rand01() - 1.0;
        x2 = 2 * Rand01() - 1.0;
        w = x1 * x1 + x2 * x2;
    } while (w > 1.0 || w == 0.0);
    w = sqrt((-2.0 * log(w)) / w);
    x1 *= w;
    x2 *= w; /* Now x1 and x2 are two indep normals (Box-Muller polar method) */
    ready = TRUE;
    return x1;
}

void TestNormalRand()
{
    int i, y, ct[10];
    real x, s, mx, mn, v;
    s = 0.0;
    v = 0.0;
    mn = HUGE;
    mx = -HUGE;
    for (i = 0; i < 10; i++)
        ct[i] = 0;
    printf("Performing 100000 randgen calls to test that normal randgen behaving ok:\n");
    for (i = 0; i < 100000; i++)
    {
        x = RandNormal();
        s += x;
        if (mx < x)
            mx = x;
        if (x < mn)
            mn = x;
        v += x * x;
        y = (int)(x * 10);
        if (x >= 0 && y < 10)
            ct[y]++;
    }
    printf("mean=%g(should be 0)  min=%f  max=%g   variance=%g(should be 1)\n",
           s / 100000.0, mn, mx, v / 100000.0);
    printf("counts in 10 bins 0-0.1, 0.1-0.2, etc: ");
    for (i = 0; i < 10; i++)
        printf(" %d", ct[i]);
    printf("\n");
    fflush(stdout);
}

/* If a 2D normal [x & y coords of which are i.i.d. standard normals]
 * is known to lie on a ray thru center; this returns the distance along the ray. 
 * Equivalently, sin(2*pi*T)*R and cos(2*pi*T)*R are iid standard normals if T is
 * random uniform in [0,1] and if R is the independent output of RandRadialNormal().  ****/
real RandRadialNormal()
{
    real w;
    do
    {
        w = Rand01();
    } while (w == 0.0);
    w = sqrt(-2.0 * log(w));
    return w;
}

void TestRadialNormalRand()
{
    int i, y, ct[10];
    real x, s, mx, mn, v;
    s = 0.0;
    v = 0.0;
    mn = HUGE;
    mx = -HUGE;
    for (i = 0; i < 10; i++)
        ct[i] = 0;
    printf("Performing 100000 randgen calls to test that radial normal randgen behaving ok:\n");
    for (i = 0; i < 100000; i++)
    {
        x = RandRadialNormal();
        s += x;
        if (mx < x)
            mx = x;
        if (x < mn)
            mn = x;
        v += x * x;
        y = (int)(x * 10);
        if (x >= 0 && y < 10)
            ct[y]++;
    }
    printf("mean=%g(should be ~1.25)  min=%f  max=%g   meansq=%g(should be 2)\n",
           s / 100000.0, mn, mx, v / 100000.0);
    printf("counts in 10 bins 0-0.1, 0.1-0.2, etc: ");
    for (i = 0; i < 10; i++)
        printf(" %d", ct[i]);
    printf("\n");
    fflush(stdout);
}

void TestRadialNormalRand2()
{
    int i, y, ct[10];
    real x, s, mx, mn, v, w;
    s = 0.0;
    v = 0.0;
    mn = HUGE;
    mx = -HUGE;
    for (i = 0; i < 10; i++)
        ct[i] = 0;
    printf("Performing 100000 randgen calls to test that normal randgen behaving ok radially:\n");
    for (i = 0; i < 100000; i++)
    {
        x = RandNormal();
        w = RandNormal();
        x = sqrt(x * x + w * w);
        s += x;
        if (mx < x)
            mx = x;
        if (x < mn)
            mn = x;
        v += x * x;
        y = (int)(x * 10);
        if (x >= 0 && y < 10)
            ct[y]++;
    }
    printf("mean=%g(should be ~1.25)  min=%f  max=%g   meansq=%g(should be 2)\n",
           s / 100000.0, mn, mx, v / 100000.0);
    printf("counts in 10 bins 0-0.1, 0.1-0.2, etc: ");
    for (i = 0; i < 10; i++)
        printf(" %d", ct[i]);
    printf("\n");
    fflush(stdout);
}

#define RECIPRTPI 0.564189583547756286948079451560772585844050 /* 1/sqrt(pi) */
real RandSkew()
{ /* returns mean=0 skewed deviate; the fact the mean is really 0, has been 
		  * confirmed by Monte Carlo to +-0.0001 */
    real x, y;
    x = RandNormal();
    y = RandNormal();
    if (x < y)
        x = y;
    return (1.21129 * (x - RECIPRTPI)); /*1.21129 chosen so that variance is 1*/
}

void GenRandNormalArr(int N, real Arr[])
{ /* returns Arr[0..N-1] of standard normal randoms */
    int i;
    for (i = 0; i < N; i++)
    {
        Arr[i] = RandNormal();
    }
}

void GenRandSkewArr(int N, real Arr[])
{ /* returns Arr[0..N-1] of skew randoms */
    int i;
    for (i = 0; i < N; i++)
    {
        Arr[i] = RandSkew();
    }
}

uint RandInt(uint N)
{ /* returns random integer in {0,1, ..., N-1} */
    return (int)(Rand01() * N);
}

real wkc(int a, int b)
{
    int si;
    assert(b > 0);
    si = (a % 2) ? 1 : -1; /*si=pseudorandom sign*/
    return si * cos((2 * a + 1) * PI / (4 * b));
}

real wks(int a, int b)
{
    assert(b > 0);
    return sin((2 * a + 1) * PI / (4 * b));
}

/* This is a goofy probability density on N-vectors.  Its mean is the zero vector.
 * The variance of each component is 1.  Its support set is all of N-space.
 * It is NOT centrosymmetric.    It has 3 or fewer modes.
 * Centrosymmetric densities (e.g. normal) have the disadvantage that they cause every Condorcet
 * voting method to be identical (and cause Condorcet winner always to exist) 
 * with distance-based utilities...  which is not realistic.  Hence this.
 **********/
#define RT85 1.264911064067351732799557417773 /* sqrt(8/5) */
#define RT25 0.632455532033675866399778708886 /* sqrt(2/5) */
void GenRandWackyArr(int N, real Arr[])
{ /* returns Arr[0..N-1] of skew randoms */
    int k, which;
    which = (int)RandInt(3);
    switch (which)
    {
    case (0):
        for (k = 0; k < N; k++)
        {
            Arr[k] = RandSkew();
        }
        break;
    case (1):
        for (k = 0; k < N; k++)
        {
            Arr[k] = wkc(k, N) + RT85 * wks(k, N) * RandNormal();
        }
        break;
    case (2):
        for (k = 0; k < N; k++)
        {
            Arr[k] = -wkc(k, N) + RT25 * wks(k, N) * RandNormal();
        }
        break;
    default:
        printf("impossible case in GenRandWackyArr\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
}

void OldSortedUpRand01Arr(int N, real Arr[])
{ /*makes Arr[0..N-1] of uniform01 randoms, SORTED increasing*/
    int i;
    real k, x, p;
    k = 1.0;
    for (i = N; i > 0; i--)
    {
        p = 1.0 / i;
        do
        {
            x = Rand01();
            x = pow(x, p);
        } while (x <= 0.0 || x >= 1.0);
        k = k * x;
        /*printf("i=%d k=%f\n", i,k);*/
        assert(k <= 1.0);
        assert(0.0 <= k);
        Arr[i - 1] = k;
    }
}

/* J.Bentley & J.Saxe: Generating Sorted Lists of Random Numbers, 
 * ACM Trans. Math'l. Software 6,3 (1980) 359-364. */
void SortedUpRand01Arr(int N, real Arr[])
{ /*makes Arr[0..N-1] of uniform01 randoms, SORTED increasing*/
    int k;
    real s;
    s = 0.0;
    for (k = 0; k < N; k++)
    {
        s -= log(Rand01());
        Arr[k] = s;
    }
    s -= log(Rand01());
    for (k = 0; k < N; k++)
    {
        Arr[k] /= s;
    }
}

void TestsOfRand()
{
    TestRand01();
    TestNormalRand();
    TestRandExpl();
    TestRadialNormalRand();
    TestRadialNormalRand2();
}

bool IsPerm(uint N, uint Perm[])
{ /* true if is a perm of [0..N-1] */
    int i;
    int ct[MaxNumCands];
    assert(N < MaxNumCands);
    for (i = 0; i < (int)N; i++)
    {
        ct[i] = 0;
    }
    for (i = 0; i < (int)N; i++)
    {
        ct[Perm[i]]++;
    }
    for (i = 0; i < (int)N; i++)
    {
        if (ct[i] != 1)
            return FALSE;
    }
    return TRUE;
}

void MakeIdentityPerm(uint N, uint Perm[])
{
    int i;
    for (i = 0; i < (int)N; i++)
    {
        Perm[i] = i;
    }
}

void RandomlyPermute(uint N, uint RandPerm[])
{ /* randomly permutes RandPerm[0..N-1] */
    int i, j, t;
    for (i = N - 1; i > 0; i--)
    {
        j = (int)RandInt((uint)i);
        t = RandPerm[j];
        RandPerm[j] = RandPerm[i];
        RandPerm[i] = t;
    }
    assert(IsPerm(N, RandPerm));
}

/******* vector handling: **********/
void CopyIntArray(uint N, int src[], int dest[])
{
    int i;
    for (i = N - 1; i >= 0; i--)
        dest[i] = src[i];
}

void CopyRealArray(uint N, real src[], real dest[])
{
    int i;
    for (i = N - 1; i >= 0; i--)
        dest[i] = src[i];
}

void FillBoolArray(uint N, bool Arr[], bool Filler)
{ /* sets Arr[0..N-1] = Filler */
    int i;
    for (i = N - 1; i >= 0; i--)
        Arr[i] = Filler;
}

void FillRealArray(uint N, real Arr[], real Filler)
{ /* sets Arr[0..N-1] = Filler */
    int i;
    for (i = N - 1; i >= 0; i--)
        Arr[i] = Filler;
}

void FillIntArray(uint N, int Arr[], int Filler)
{ /* sets Arr[0..N-1] = Filler */
    int i;
    for (i = N - 1; i >= 0; i--)
        Arr[i] = Filler;
}

void ZeroIntArray(uint N, int Arr[])
{ /* sets Arr[0..N-1] = 0 */
    FillIntArray(N, Arr, 0);
}

void ZeroRealArray(uint N, real Arr[])
{ /* sets Arr[0..N-1] = 0 */
    FillRealArray(N, Arr, 0.0);
}

/* Assumes RandPerm[0..N-1] contains perm. Returns index of random min entry of Arr[0..N-1]. */
int ArgMinIntArr(uint N, int Arr[], int RandPerm[])
{
    int minc, i, r, winner;
    winner = -1;
    minc = BIGINT;
    RandomlyPermute(N, (uint *)RandPerm);
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] < minc)
        {
            minc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    assert(Arr[winner] <= Arr[0]);
    assert(Arr[winner] <= Arr[N - 1]);
    return (winner);
}

/* Assumes RandPerm[0..N-1] contains perm. Returns index of random max entry of Arr[0..N-1]. */
int ArgMaxIntArr(uint N, int Arr[], int RandPerm[])
{
    int maxc, i, r, winner;
    winner = -1;
    maxc = -BIGINT;
    RandomlyPermute(N, (uint *)RandPerm);
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] > maxc)
        {
            maxc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    return (winner);
}

/* Assumes RandPerm[0..N-1] contains perm. Returns index of random max entry of Arr[0..N-1]. */
int ArgMaxUIntArr(uint N, uint Arr[], int RandPerm[])
{
    uint maxc;
    int i, r, winner;
    winner = -1;
    maxc = 0;
    RandomlyPermute(N, (uint *)RandPerm);
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] >= maxc)
        {
            maxc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    return (winner);
}

/* Assumes RandPerm[0..N-1] contains perm. Returns index of random min entry of Arr[0..N-1]. */
int ArgMinRealArr(uint N, real Arr[], int RandPerm[])
{
    real minc;
    int i, r, winner;
    winner = -1;
    minc = HUGE;
    RandomlyPermute(N, (uint *)RandPerm);
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] < minc)
        {
            minc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    return (winner);
}

/* Assumes RandPerm[0..N-1] contains perm. Returns index of random max entry of Arr[0..N-1]. */
int ArgMaxRealArr(uint N, real Arr[], int RandPerm[])
{
    real maxc;
    int i, r, winner;
    winner = -1;
    maxc = -HUGE;
    RandomlyPermute(N, (uint *)RandPerm);
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] > maxc)
        {
            maxc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    return (winner);
}

/* Assumes RandPerm[0..N-1] contains random perm and MinInd is index for Arr[] yielding min value.
 * Returns index of second-min. */
int Arg2MinIntArr(uint N, int Arr[], int RandPerm[], int MinInd)
{
    int minc, i, r, winner;
    winner = -1;
    minc = BIGINT;
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] < minc && r != MinInd)
        {
            minc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    return (winner);
}

/* Assumes RandPerm[0..N-1] contains random perm and MaxInd is index for Arr[] yielding max.
 * Returns index of second-max. */
int Arg2MaxIntArr(uint N, int Arr[], int RandPerm[], int MaxInd)
{
    int maxc, i, r, winner;
    winner = -1;
    maxc = -BIGINT;
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] > maxc && r != MaxInd)
        {
            maxc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    return (winner);
}

/* Assumes RandPerm[0..N-1] contains random perm and MaxInd is index for Arr[] yielding max.
 * Returns index of second-max. */
int Arg2MaxUIntArr(uint N, uint Arr[], int RandPerm[], int MaxInd)
{
    uint maxc;
    int i, r, winner;
    winner = -1;
    maxc = 0;
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] >= maxc && r != MaxInd)
        {
            maxc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    return (winner);
}

/* Assumes RandPerm[0..N-1] contains random perm and MaxInd is index for Arr[] yielding max.
 * Returns index of second-max. */
int Arg2MaxRealArr(uint N, real Arr[], int RandPerm[], int MaxInd)
{
    real maxc;
    int i, r, winner;
    winner = -1;
    maxc = -HUGE;
    for (i = 0; i < (int)N; i++)
    {
        r = RandPerm[i];
        if (Arr[r] > maxc && r != MaxInd)
        {
            maxc = Arr[r];
            winner = r;
        }
    }
    assert(winner >= 0);
    return (winner);
}

void ScaleRealVec(uint N, real a[], real scalefac)
{
    int i;
    for (i = 0; i < (int)N; i++)
        a[i] *= scalefac;
}

void ScaleIntVec(uint N, int a[], int scalefac)
{
    int i;
    for (i = 0; i < (int)N; i++)
        a[i] *= scalefac;
}

/*Donald Knuth's "The Art of Computer Programming, Volume 2: Seminumerical Algorithms", section 4.2.2
describes how to compute mean and standard deviation using a recurrence relation, like this:
M(1) = x(1),   M(k) = M(k-1) + (x(k) - M(k-1)) / k
S(1) = 0,      S(k) = S(k-1) + (x(k) - M(k-1)) * (x(k) - M(k))
for 2 <= k <= n, then
sigma = sqrt(S(n) / (n - 1))
Attributes this method to B.P. Welford, Technometrics 4 (1962) 419-420.
*******/
__attribute__((always_inline)) inline void WelfordUpdateMeanSD(real NewDatum, int *Count, real *M, real *S)
//inline void WelfordUpdateMeanSD(real NewDatum, int *Count, real *M, real *S)
{
    real OldMean;
    OldMean = *M;
    (*Count)++;
    *M += (NewDatum - OldMean) / (*Count);
    *S += (NewDatum - OldMean) * (NewDatum - *M);
    return;
}

real DistanceSquared(uint N, real a[], real b[])
{
    real d = 0.0;
    int i;
    for (i = 0; i < (int)N; i++)
        d += SquareReal(a[i] - b[i]);
    return d;
}

real L1Distance(uint N, real a[], real b[])
{
    real d = 0.0;
    int i;
    for (i = 0; i < (int)N; i++)
        d += fabs(a[i] - b[i]);
    return d;
}

real LpDistanceSquared(uint N, real a[], real b[], real Lp)
{
    real d = 0.0;
    int i;
    assert(Lp >= 1.0);
    if (Lp == 1.0)
        return SquareReal(L1Distance(N, a, b));
    if (Lp == 2.0)
        return DistanceSquared(N, a, b);
    for (i = 0; i < (int)N; i++)
        d += pow(fabs(a[i] - b[i]), Lp);
    return pow(d, 2.0 / Lp);
}

real LpDistance(uint N, real a[], real b[], real Lp)
{
    real d = 0.0;
    int i;
    assert(Lp >= 1.0);
    if (Lp == 1.0)
        return L1Distance(N, a, b);
    if (Lp == 2.0)
        return sqrt(DistanceSquared(N, a, b));
    for (i = 0; i < (int)N; i++)
        d += pow(fabs(a[i] - b[i]), Lp);
    return pow(d, 1.0 / Lp);
}

real SumRealArray(uint N, real a[])
{
    real s = 0.0;
    int i;
    for (i = 0; i < (int)N; i++)
        s += a[i];
    return s;
}

real DotProd(uint N, real a[], real b[])
{
    real d = 0.0;
    int i;
    for (i = 0; i < (int)N; i++)
        d += a[i] * b[i];
    return d;
}

/******* sorting: **********/
int SortedReal(uint N, real Arr[])
{ /* +1 if Arr[0..N-1] increasing, -1 if decreasing, 2 if unsorted, 0 if all-eq */
    int i, s;
    s = SignReal(Arr[N - 1] - Arr[0]);
    for (i = 1; i < (int)N; i++)
    {
        if (s * SignReal(Arr[i] - Arr[i - 1]) < 0)
            return (2);
    }
    return s;
}

int SortedInt(uint N, int Arr[])
{ /* +1 if Arr[0..N-1] increasing, -1 if decreasing, 2 if unsorted, 0 if all-eq */
    int i, s;
    s = SignInt(Arr[N - 1] - Arr[0]);
    for (i = 1; i < (int)N; i++)
    {
        if (s * SignInt(Arr[i] - Arr[i - 1]) < 0)
            return (2);
    }
    return s;
}

int SortedRealKey(uint N, int Arr[], real Key[])
{
    int i, s;
    s = SignReal(Key[Arr[N - 1]] - Key[Arr[0]]);
    for (i = 1; i < (int)N; i++)
    {
        if (s * SignReal(Key[Arr[i]] - Key[Arr[i - 1]]) < 0)
            return (2);
    }
    return s;
}

int SortedIntKey(uint N, int Arr[], int Key[])
{
    int i, s;
    s = SignInt(Key[Arr[N - 1]] - Key[Arr[0]]);
    for (i = 1; i < (int)N; i++)
    {
        if (s * SignInt(Key[Arr[i]] - Key[Arr[i - 1]]) < 0)
            return (2);
    }
    return s;
}

const int ShellIncs[] = {1750, 701, 301, 132, 57, 23, 10, 4, 1, 0};
/* Marcin Ciura: Best Increments for the Average Case of Shellsort,
   13th International Symposium on Fundamentals of Computation Theory,
   Riga, Latvia, 22-24 August 2001;
   Springer Lecture Notes in Computer Science #2138, pp.106-117. 
Here 1750 is unsure and how the sequence continues past 1750 is unknown.
 ***/

void RealShellSortUp(uint N, real Arr[])
{ /* Sorts Arr[0..N-1] into increasing order */
    int h, i, j, k;
    real x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {
        for (i = h; i < (int)N; i++)
        {
            x = Arr[i];
            for (j = i - h; j >= 0 && Arr[j] > x; j -= h)
            {
                Arr[j + h] = Arr[j];
            }
            Arr[j + h] = x;
        }
    }
    assert((SortedReal(N, Arr) & (~1)) == 0);
}

void IntShellSortUp(uint N, int Arr[])
{ /* Sorts Arr[0..N-1] into increasing order */
    int h, i, j, k;
    int x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {

        for (i = h; i < (int)N; i++)
        {
            x = Arr[i];
            for (j = i - h; j >= 0 && Arr[j] > x; j -= h)
            {
                Arr[j + h] = Arr[j];
            }
            Arr[j + h] = x;
        }
    }
    assert((SortedInt(N, Arr) & (~1)) == 0);
}

void RealShellSortDown(uint N, real Arr[])
{ /* Sorts Arr[0..N-1] into decreasing order */
    int h, i, j, k;
    real x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {
        for (i = h; i < (int)N; i++)
        {
            x = Arr[i];
            for (j = i - h; j >= 0 && Arr[j] < x; j -= h)
            {
                Arr[j + h] = Arr[j];
            }
            Arr[j + h] = x;
        }
    }
    assert(SortedReal(N, Arr) <= 0);
}

void IntShellSortDown(uint N, int Arr[])
{ /* Sorts Arr[0..N-1] into decreasing order */
    int h, i, j, k;
    int x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {
        for (i = h; i < (int)N; i++)
        {
            x = Arr[i];
            for (j = i - h; j >= 0 && Arr[j] < x; j -= h)
            {
                Arr[j + h] = Arr[j];
            }
            Arr[j + h] = x;
        }
    }
    assert(SortedInt(N, Arr) <= 0);
}

/* Rearranges Perm[0..N-1] so Key[Perm[0..N-1]] is in increasing order: */
void IntPermShellSortUp(uint N, int Perm[], int Key[])
{
    int h, i, j, k;
    int x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {
        for (i = h; i < (int)N; i++)
        {
            x = Perm[i];
            for (j = i - h; j >= 0 && Key[Perm[j]] > Key[x]; j -= h)
            {
                Perm[j + h] = Perm[j];
            }
            Perm[j + h] = x;
        }
    }
    assert((SortedIntKey(N, Perm, Key) & (~1)) == 0);
}

/* Rearranges Perm[0..N-1] so Key[Perm[0..N-1]] is in increasing order: */
void RealPermShellSortUp(uint N, int Perm[], real Key[])
{
    int h, i, j, k;
    int x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {
        for (i = h; i < (int)N; i++)
        {
            x = Perm[i];
            for (j = i - h; j >= 0 && Key[Perm[j]] > Key[x]; j -= h)
            {
                Perm[j + h] = Perm[j];
            }
            Perm[j + h] = x;
        }
    }
    assert((SortedRealKey(N, Perm, Key) & (~1)) == 0);
}

/* Rearranges Perm[0..N-1] so Key[Perm[0..N-1]] is in decreasing order.  Key[] not altered: */
void IntPermShellSortDown(uint N, int Perm[], int Key[])
{
    int h, i, j, k;
    int x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {
        for (i = h; i < (int)N; i++)
        {
            x = Perm[i];
            for (j = i - h; j >= 0 && Key[Perm[j]] < Key[x]; j -= h)
            {
                Perm[j + h] = Perm[j];
            }
            Perm[j + h] = x;
        }
    }
    assert(SortedIntKey(N, Perm, Key) <= 0);
}

/* Rearranges Perm[0..N-1] so Key[Perm[0..N-1]] is in decreasing order.  Key[] not altered: */
void ScharPermShellSortDown(uint N, schar Perm[], uint Key[])
{
    int h, i, j, k;
    int x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {
        for (i = h; i < (int)N; i++)
        {
            x = Perm[i];
            for (j = i - h; j >= 0 && Key[Perm[j]] < Key[x]; j -= h)
            {
                Perm[j + h] = Perm[j];
            }
            Perm[j + h] = x;
        }
    }
}

/* Rearranges Perm[0..N-1] so Key[Perm[0..N-1]] is in decreasing order: */
void RealPermShellSortDown(uint N, int Perm[], real Key[])
{
    int h, i, j, k;
    int x;
    for (k = 0; (h = ShellIncs[k]) > 0; k++)
    {
        for (i = h; i < (int)N; i++)
        {
            x = Perm[i];
            for (j = i - h; j >= 0 && Key[Perm[j]] < Key[x]; j -= h)
            {
                Perm[j + h] = Perm[j];
            }
            Perm[j + h] = x;
        }
    }
    assert(SortedRealKey(N, Perm, Key) <= 0);
}

bool SelectedRightInt(uint L, uint R, uint K, int A[])
{
    uint i;
    for (i = L; i < K; i++)
    {
        if (A[i] > A[K])
            return FALSE;
    }
    for (i = R; i > K; i--)
    {
        if (A[K] > A[i])
            return FALSE;
    }
    return TRUE;
}

bool SelectedRightReal(uint L, uint R, uint K, real A[])
{
    uint i;
    for (i = L; i < K; i++)
    {
        if (A[i] > A[K])
            return FALSE;
    }
    for (i = R; i > K; i--)
    {
        if (A[K] > A[i])
            return FALSE;
    }
    return TRUE;
}

/* Rearranges A[L..R] so that A[i] <= A[K] <= A[j]  if L<=i<K<j<=R.
 * Then returns A[K].  If R-L+1=N then
 * expected runtime for randomly ordered A[] is 1.5*N compares 
 * asymptotically to find median (and less for other K, finds max in only 1.0*N).
 * In particular if L=0, R=N-1 and K=(N-1)/2 with N odd, then returns median.
 * If N is even, then use K=N/2 to find the upper bimedian, and then
 * the max of A[0..K-1] is the lower bimedian. 
 *   Note: the magic constants 601, 0.5, 0.5, 20, 5, and 5 in the below should
 * be tuned to optimize speed.  They are probably improveable.
 ***/
int FloydRivestSelectInt(uint L, uint R, uint K, int A[])
{
    int I, J, N, S, SD, LL, RR;
    real Z;
    int T, W;
    while (R > L)
    {
        N = R - L + 1;
        if (N > 601)
        { /* big enough so worth doing FR-type subsampling to find strong splitter */
            I = K - L + 1;
            Z = log((real)(N));
            S = (int)(0.5 * exp(Z * 2.0 / 3));
            SD = (int)(0.5 * sqrt(Z * S * (N - S) / N) * SignInt(2 * I - N));
            LL = MaxInt(L, K - ((I * S) / N) + SD);
            RR = MinInt(R, K + (((N - I) * S) / N) + SD);
            /* Recursively select inside small subsample to find an element A[K] usually very 
       * near the desired one: */
            FloydRivestSelectInt(LL, RR, K, A);
        }
        else if (N > 20 && (int)(5 * (K - L)) > N && (int)(5 * (R - K)) > N)
        {
            /* not big enough to be worth evaluating those expensive logs etc; 
         * but big enough so random splitter is poor; so use median-of-3 */
            I = K - 1;
            if (A[K] < A[I])
            {
                W = A[I];
                A[I] = A[K];
                A[K] = W;
            }
            I = K + 1;
            if (A[I] < A[K])
            {
                W = A[I];
                A[I] = A[K];
                A[K] = W;
            }
            I = K - 1;
            if (A[K] < A[I])
            {
                W = A[I];
                A[I] = A[K];
                A[K] = W;
            }
        } /* otherwise using random splitter (i.e. current value of A[K]) */
        /* now use A[K] to split A[L..R] into two parts... */
        T = A[K];
        I = L;
        J = R;
        W = A[L];
        A[L] = A[K];
        A[K] = W;
        if (A[R] > T)
        {
            W = A[R];
            A[R] = A[L];
            A[L] = W;
        }
        while (I < J)
        {
            W = A[I];
            A[I] = A[J];
            A[J] = W;
            I++;
            J--;
            while (A[I] < T)
            {
                I++;
            }
            while (A[J] > T)
            {
                J--;
            }
        }
        if (A[L] == T)
        {
            W = A[L];
            A[L] = A[J];
            A[J] = W;
        }
        else
        {
            J++;
            W = A[J];
            A[J] = A[R];
            A[R] = W;
        }
        if (J <= (int)K)
        {
            L = J + 1;
        }
        if ((int)K <= J)
        {
            R = J - 1;
        }
        /* Now continue on using contracted [L..R] interval... */
    }
    return (A[K]);
}

/* returns twice the median of A[0..N-1] or sum of the bimedians if N even */
int TwiceMedianInt(uint N, int A[])
{
    int M, T;
    int i;
    assert(N > 0);
    M = FloydRivestSelectInt(0, N - 1, N / 2, A);
    assert(SelectedRightInt(0, N - 1, N / 2, A));
    if ((N & 1) == 0)
    { /*N is even*/
        T = A[N / 2 - 1];
        for (i = N / 2 - 2; i >= 0; i--)
        {
            if (A[i] > T)
                T = A[i];
        }
    }
    else
    {
        T = M;
    }
    return T + M;
}

/* Rearranges A[L..R] so that A[i] <= A[K] <= A[j]  if L<=i<K<j<=R.
 * Then returns A[K].  If R-L+1=N then
 * expected runtime for randomly ordered A[] is 1.5*N compares 
 * asymptotically to find median (and less for other K, finds max in only 1.0*N).
 * In particular if L=0, R=N-1 and K=(N-1)/2 with N odd, then returns median.
 * If N is even, then use K=N/2 to find the upper bimedian, and then
 * the max of A[0..K-1] is the lower bimedian. 
 ***/
real FloydRivestSelectReal(uint L, uint R, uint K, real A[])
{
    int I, J, N, S, SD, LL, RR;
    real Z;
    real T, W;
    while (R > L)
    {
        N = R - L + 1;
        if (N > 601)
        { /* big enough so worth doing FR-type subsampling to find strong splitter */
            I = K - L + 1;
            Z = log((real)(N));
            S = (int)(0.5 * exp(Z * 2.0 / 3));
            SD = (int)(0.5 * sqrt(Z * S * (N - S) / N) * SignInt(2 * I - N));
            LL = MaxInt(L, K - ((I * S) / N) + SD);
            RR = MinInt(R, K + (((N - I) * S) / N) + SD);
            /* Recursively select inside small subsample to find an element A[K] usually very 
       * near the desired one: */
            FloydRivestSelectReal(LL, RR, K, A);
        }
        else if (N > 20 && (int)(5 * (K - L)) > N && (int)(5 * (R - K)) > N)
        {
            /* not big enough to be worth evaluating those expensive logs etc; 
         * but big enough so random splitter is poor; so use median-of-3 */
            I = K - 1;
            if (A[K] < A[I])
            {
                W = A[I];
                A[I] = A[K];
                A[K] = W;
            }
            I = K + 1;
            if (A[I] < A[K])
            {
                W = A[I];
                A[I] = A[K];
                A[K] = W;
            }
            I = K - 1;
            if (A[K] < A[I])
            {
                W = A[I];
                A[I] = A[K];
                A[K] = W;
            }
        } /* otherwise using random splitter (i.e. current value of A[K]) */
        /* now use A[K] to split A[L..R] into two parts... */
        T = A[K];
        I = L;
        J = R;
        W = A[L];
        A[L] = A[K];
        A[K] = W;
        if (A[R] > T)
        {
            W = A[R];
            A[R] = A[L];
            A[L] = W;
        }
        while (I < J)
        {
            W = A[I];
            A[I] = A[J];
            A[J] = W;
            I++;
            J--;
            while (A[I] < T)
            {
                I++;
            }
            while (A[J] > T)
            {
                J--;
            }
        }
        if (A[L] == T)
        {
            W = A[L];
            A[L] = A[J];
            A[J] = W;
        }
        else
        {
            J++;
            W = A[J];
            A[J] = A[R];
            A[R] = W;
        }
        if (J <= (int)K)
        {
            L = J + 1;
        }
        if ((int)K <= J)
        {
            R = J - 1;
        }
        /* Now continue on using contracted [L..R] interval... */
    }
    return (A[K]);
}

/* returns twice the median of A[0..N-1] or sum of the bimedians if N even */
real TwiceMedianReal(uint N, real A[])
{
    real M, T;
    int i;
    assert(N > 0);
    M = FloydRivestSelectReal(0, N - 1, N / 2, A);
    assert(SelectedRightReal(0, N - 1, N / 2, A));
    if ((N & 1) == 0)
    { /*N is even*/
        T = A[N / 2 - 1];
        for (i = N / 2 - 2; i >= 0; i--)
        {
            if (A[i] > T)
                T = A[i];
        }
    }
    else
    {
        T = M;
    }
    return T + M;
}

/*************************** VOTING METHODS:  ********
all are subroutines with a common format - here is the format (which is all subsumed in
the convenient data structure "edata"):
input:
uint NumVoters;
uint NumCands;

uint TopDownPrefs[NumVoters*NumCands];   
Entry x*NumCands+y says the candidate who is the (y-1)th choice of voter x, x=0..NumVoters-1.
Candidates are numbered 0..NumCands-1.

uint CandRankings[NumVoters*NumCands];
Entry x*NumCands+y says the ranking (0=top) of the yth candidate (y=0..NumCands-1)
according to voter x, x=0..NumVoters-1.

real Score[NumVoters*NumCands];
Entry x*NumCands+y says the score (a floating point real from 0 to 1)
of the yth candidate (y=0..NumCands-1) according to voter x, x=0..NumVoters-1.

bool Approve[NumVoters*NumCands];
Entry x*NumCands+y says the Approval (a boolean)
of the yth candidate (y=0..NumCands-1) according to voter x, x=0..NumVoters-1.

bool Approve2[NumVoters*NumCands];
Entry x*NumCands+y says the second-level Approval (a boolean)
of the yth candidate (y=0..NumCands-1) according to voter x, x=0..NumVoters-1.
Used in MCA.  A higher level of approval.

real PerceivedUtility[NumVoters*NumCands];
Entry x*NumCands+y says the utility (a floating point real)
of the yth candidate (y=0..NumCands-1) according to voter x, x=0..NumVoters-1.
Note, this is "honest", i.e. undistorted by strategy, 
although it IS distorted by ignorance.

real Utility[MaxNumCands*MaxNumVoters];
Entry x*NumCands+y says the utility (a floating point real)
of the yth candidate (y=0..NumCands-1) according to voter x, x=0..NumVoters-1.
Note, this is "honest", i.e. undistorted by strategy, AND undistorted by ignorance.

semi-input:  (used as input by some election methods, but computed from the preceding
genuine input by BuildDefeatsMatrix)
int DefeatsMatrix[MaxNumCands*MaxNumCands]; 
DefeatsMatrix[i*NumCands+j] is #voters who ranked i above j  (always nonnegative)

real Armytage[MaxNumCands*MaxNumCands]; 
Armytage[i*NumCands+j] is sum of rating[i]-rating[j] for voters who rated i above j  (always nonnegative)

int MarginsMatrix[MaxNumCands*MaxNumCands]; 
MarginsMatrix[i*NumCands+j] is margin of i over j  (either sign)

real MargArmy[MaxNumCands*MaxNumCands]; 
MargArmy[i*NumCands+j] is Armytage[i*NumCands+j]-Armytage[j*NumCands+i]


output (the returned value):
int Winner;       Number in {0,1,...,NumCands-1}  saying who won. 
                  (Negative number indicates error condition.)
                  Note: tie breaking is random.  A global array RandPerm[0..NumCands-1] is
                  assumed available to help with that task.

CORE METHODS: Plurality, Borda, IRV, SociallyBest, SociallyWorst, RandomWinner, Approval, Range, 
  SmithSet, SchwartzSet, Antiplurality
are "core" voting methods which must always be run (and first), otherwise
some other methods may not work.

side-effects:  can alter these global variables (list follows):
***************************/

int PlurWinner;
int AntiPlurWinner;
int PSecond;
int RSecond;
int ASecond;
int ApprovalWinner;
int IRVwinner;
int SmithWinner;
int RandWinner;
int SchwartzWinner;
int RangeWinner;
int BordaWinner;
int WorstWinner;
int BestWinner;
int RandomUncoveredMemb;
uint RangeGranul;
int IRVTopLim = BIGINT;
int CondorcetWinner; /*negative if does not exist*/
int TrueCW;          /*cond winner based on undistorted true utilities; negative if does not exist*/
int CopelandWinner;
int CopeWinOnlyWinner;
int SmithIRVwinner;
uint PlurVoteCount[MaxNumCands];
uint AntiPlurVoteCount[MaxNumCands];
uint DabaghVoteCount[MaxNumCands];
int VFAVoteCount[MaxNumCands];
int RdVoteCount[MaxNumCands];
int FavListNext[MaxNumVoters];
int HeadFav[MaxNumCands];
int WinCount[MaxNumCands];
int DrawCt[MaxNumCands];
int CopeScore[MaxNumCands];
int LossCount[MaxNumCands];
int SimmVotesAgainst[MaxNumCands];
int BeatPathStrength[MaxNumCands * MaxNumCands];
real ArmyBPS[MaxNumCands * MaxNumCands];
uint ApprovalVoteCount[MaxNumCands];
int UncAAOF[MaxNumCands];
uint MCAVoteCount[MaxNumCands];
real RangeVoteCount[MaxNumCands];
real SumNormedRating[MaxNumCands];
uint RangeNVoteCount[MaxNumCands];
real CCumVoteCount[MaxNumCands];
real MedianRating[MaxNumCands];
real CScoreVec[MaxNumVoters];
int MedianRank[MaxNumCands];
int CRankVec[MaxNumVoters];
uint BordaVoteCount[MaxNumCands];
int NansonVoteCount[MaxNumCands];
uint NauruVoteCount[MaxNumCands];
uint HeismanVoteCount[MaxNumCands];
uint BaseballVoteCount[MaxNumCands];
uint SumOfDefeatMargins[MaxNumCands];
int WorstDefeatMargin[MaxNumCands];
int RayDefeatMargin[MaxNumCands];
int RayBeater[MaxNumCands];
int ARVictMargin[MaxNumCands];
int ARchump[MaxNumCands];
real UtilitySum[MaxNumCands];
real UtilityRootSum[MaxNumCands];
uint RandCandPerm[MaxNumCands]; /* should initially contain 0..NumCands-1 */
bool MajApproved[MaxNumCands];
bool Eliminated[MaxNumCands];
bool MDdisquald[MaxNumCands];
bool BSSmithMembs[MaxNumCands];
bool SmithMembs[MaxNumCands];
bool UncoveredSt[MaxNumCands];
bool SchwartzMembs[MaxNumCands];
int IFav[MaxNumVoters];
uint NauruWt[MaxNumCands];
uint BaseballWt[MaxNumCands];
int PairApproval[MaxNumCands * MaxNumCands];
real SinkRat[MaxNumCands], SinkRow[MaxNumCands], SinkCol[MaxNumCands];
real SinkMat[MaxNumCands * MaxNumCands];
bool CoverMatrix[MaxNumCands * MaxNumCands];
real EigVec[MaxNumCands], EigVec2[MaxNumCands];
bool Rmark[MaxNumCands];
bool rRmark[MaxNumCands];
int summ[MaxNumCands];
int Tpath[MaxNumCands * MaxNumCands];
int Hpotpar[MaxNumCands];
int Hpar[MaxNumCands];
int Hroot[MaxNumCands];
uint WoodHashCount[3 * MaxNumCands * MaxNumVoters], WoodHashSet[3 * MaxNumCands * MaxNumVoters];
uint WoodSetPerm[3 * MaxNumCands * MaxNumVoters];
uint IBeat[MaxNumCands];

void InitCoreElState()
{ /*can use these flags to tell if Plurality() etc have been run*/
    PlurWinner = -1;
    BordaWinner = -1;
    ApprovalWinner = -1;
    RangeWinner = -1;
    IRVwinner = -1;
    BestWinner = -1;
    WorstWinner = -1;
    AntiPlurWinner = -1;
    PSecond = -1;
    RandWinner = -1;
    SmithWinner = -1;
    SchwartzWinner = -1;
    CopeWinOnlyWinner = -1;
    SmithIRVwinner = -1;
    RandomUncoveredMemb = -1;
    IRVTopLim = BIGINT;
}

typedef struct dum1
{
    uint NumVoters;
    uint NumCands;
    uint TopDownPrefs[MaxNumCands * MaxNumVoters];
    uint CandRankings[MaxNumCands * MaxNumVoters];
    real Score[MaxNumCands * MaxNumVoters];
    bool Approve[MaxNumCands * MaxNumVoters];
    bool Approve2[MaxNumCands * MaxNumVoters];
    real PerceivedUtility[MaxNumCands * MaxNumVoters];
    real Utility[MaxNumCands * MaxNumVoters];
    int TrueDefMatrix[MaxNumCands * MaxNumCands];
    int DefeatsMatrix[MaxNumCands * MaxNumCands];
    int MarginsMatrix[MaxNumCands * MaxNumCands];
    real Armytage[MaxNumCands * MaxNumCands];
    int ArmyDef[MaxNumCands * MaxNumCands];
    real MargArmy[MaxNumCands * MaxNumCands];
} edata;

void PrintEdata(FILE *F, edata *E)
{ /* prints out the edata */
    int v, j;
    fprintf(F, "NumVoters=%d  NumCands=%d\n", E->NumVoters, E->NumCands);
    for (v = 0; v < (int)E->NumVoters; v++)
    {
        fprintf(F, "Voter %2d:\n", v);
        fprintf(F, "Utility: ");
        for (j = 0; j < E->NumCands; j++)
        {
            fprintf(F, "%6.3f", E->Utility[v * E->NumCands + j]);
        }
        fprintf(F, "\n");
        fprintf(F, "PercUti: ");
        for (j = 0; j < E->NumCands; j++)
        {
            fprintf(F, "%6.3f", E->PerceivedUtility[v * E->NumCands + j]);
        }
        fprintf(F, "\n");
        fprintf(F, "RangeScore: ");
        for (j = 0; j < E->NumCands; j++)
        {
            fprintf(F, "%6.3f", E->Score[v * E->NumCands + j]);
        }
        fprintf(F, "\n");
        fprintf(F, "CandRank: ");
        for (j = 0; j < E->NumCands; j++)
        {
            fprintf(F, "%2d", E->CandRankings[v * E->NumCands + j]);
        }
        fprintf(F, "\n");
        fprintf(F, "TopDown: ");
        for (j = 0; j < E->NumCands; j++)
        {
            fprintf(F, "%2d", E->TopDownPrefs[v * E->NumCands + j]);
        }
        fprintf(F, "\n");
        fprintf(F, "Approve: ");
        for (j = 0; j < E->NumCands; j++)
        {
            fprintf(F, "%2d", E->Approve[v * E->NumCands + j] ? 1 : 0);
        }
        fprintf(F, "\n");
        fprintf(F, "Approve2: ");
        for (j = 0; j < E->NumCands; j++)
        {
            fprintf(F, "%2d", E->Approve2[v * E->NumCands + j] ? 1 : 0);
        }
        fprintf(F, "\n");
    }
    /*???more?*/
    fflush(F);
}

#define EMETH int /* allows fgrep EMETH IEVS.c to find out what Election methods now available */

void BuildDefeatsMatrix(edata *E)
{ /* initializes  E->DefeatsMatrix[], E->MarginsMatrix[], RandCandPerm[], NauruWt[], WinCount[], DrawCt[], CondorcetWinner, CopeWinOnlyWinner, TrueCW */
    int k, i, j, y;
    uint x;
    real t;
    bool CondWin, TrueCondWin;
    MakeIdentityPerm(E->NumCands, RandCandPerm);
    RandomlyPermute(E->NumCands, RandCandPerm);

    assert(E->NumCands <= MaxNumCands);
    x = LCMfact[E->NumCands];
    for (j = E->NumCands - 1; j >= 0; j--)
    {
        NauruWt[j] = x / (j + 1);
    }
    for (j = MinInt(E->NumCands - 1, 9); j >= 0; j--)
    {
        BaseballWt[j] = 10 - j;
    }
    BaseballWt[0] = 14;
    ZeroIntArray(SquareInt(E->NumCands), E->DefeatsMatrix);
    ZeroRealArray(SquareInt(E->NumCands), E->Armytage);
    ZeroIntArray(SquareInt(E->NumCands), E->ArmyDef);
    ZeroIntArray(SquareInt(E->NumCands), E->TrueDefMatrix);
    for (k = 0; k < (int)E->NumVoters; k++)
    {
        x = k * E->NumCands;
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            for (j = i - 1; j >= 0; j--)
            {
                y = E->CandRankings[x + j];
                y -= E->CandRankings[x + i];
                if (y > 0)
                {
                    E->DefeatsMatrix[i * E->NumCands + j]++; /*i preferred above j*/
                }
                else
                {
                    E->DefeatsMatrix[j * E->NumCands + i]++;
                }
                t = E->Score[x + i] - E->Score[x + j];
                if (t > 0.0)
                {
                    E->Armytage[i * E->NumCands + j] += t; /*i preferred above j*/
                    E->ArmyDef[i * E->NumCands + j]++;
                }
                else
                {
                    E->Armytage[j * E->NumCands + i] -= t;
                    E->ArmyDef[j * E->NumCands + i]++;
                }
                t = E->Utility[x + i] - E->Utility[x + j];
                if (t > 0.0)
                {
                    E->TrueDefMatrix[i * E->NumCands + j]++;
                }
                else
                {
                    E->TrueDefMatrix[j * E->NumCands + i]++;
                }
            }
        }
    }
    CondorcetWinner = -1;
    TrueCW = -1;
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        WinCount[i] = DrawCt[i] = 0;
        CondWin = TRUE;
        TrueCondWin = TRUE;
        for (j = (int)E->NumCands - 1; j >= 0; j--)
        {
            assert(E->DefeatsMatrix[i * E->NumCands + j] <= (int)E->NumVoters);
            assert(E->DefeatsMatrix[i * E->NumCands + j] >= 0);
            assert(E->DefeatsMatrix[i * E->NumCands + j] + E->DefeatsMatrix[j * E->NumCands + i] <= (int)E->NumVoters);
            y = E->ArmyDef[i * E->NumCands + j];
            y -= E->ArmyDef[j * E->NumCands + i];
            if (y > 0)
                E->MargArmy[i * E->NumCands + j] = E->Armytage[i * E->NumCands + j];
            else /*y<=0*/
                E->MargArmy[i * E->NumCands + j] = 0;
            y = E->DefeatsMatrix[i * E->NumCands + j];
            y -= E->DefeatsMatrix[j * E->NumCands + i];
            E->MarginsMatrix[i * E->NumCands + j] = y;
            assert(i != j || E->MarginsMatrix[i * E->NumCands + j] == 0);
            if (y > 0)
                WinCount[i]++;
            if (y == 0)
                DrawCt[i]++;
            if (y <= 0 && j != i)
            {
                CondWin = FALSE;
            } /* if beaten or tied, not a CondorcetWinner by this defn */
            y = E->TrueDefMatrix[i * E->NumCands + j];
            y -= E->TrueDefMatrix[j * E->NumCands + i];
            if (y <= 0 && j != i)
            {
                TrueCondWin = FALSE;
            }
        }
        if (CondWin)
            CondorcetWinner = i; /* i beats all opponents (unique Condorcet winner) */
        if (TrueCondWin)
            TrueCW = i; /* i beats all opponents (unique Condorcet winner) */
    }
    /* find who-you-beat sets: */
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        x = 0;
        for (j = (int)E->NumCands - 1; j >= 0; j--)
        {
            if (E->MarginsMatrix[i * E->NumCands + j] > 0)
            {
                x |= (1 << j);
            }
        }
        IBeat[i] = x;
    }
    CopeWinOnlyWinner = ArgMaxIntArr(E->NumCands, WinCount, (int *)RandCandPerm);
    ZeroIntArray(SquareInt(E->NumCands), PairApproval);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            for (k = E->NumCands - 1; k > j; k--)
            {
                y = ((E->Approve[x + j] && E->Approve[x + k]) ? 1 : 0);
                PairApproval[j * E->NumCands + k] += y; /* count of voters who approve of both j and k */
                PairApproval[k * E->NumCands + j] += y;
            }
        }
    }
}

EMETH SociallyBest(edata *E /* greatest utility-sum winner */
)
{ /* side effects: UtilitySum[], BestWinner */
    uint x;
    int i, j;
    ZeroRealArray(E->NumCands, UtilitySum);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            UtilitySum[j] += E->Utility[x + j];
        }
    }
    BestWinner = ArgMaxRealArr(E->NumCands, UtilitySum, (int *)RandCandPerm);
    for (j = E->NumCands - 1; j >= 0; j--)
    {
        assert(UtilitySum[BestWinner] >= UtilitySum[j]);
    }
    return BestWinner;
}

EMETH SociallyWorst(edata *E /* least utility-sum winner */
)
{ /* side effects: UtilitySum[], WorstWinner */
    if (BestWinner < 0)
        SociallyBest(E);
    WorstWinner = ArgMinRealArr(E->NumCands, UtilitySum, (int *)RandCandPerm);
    return WorstWinner;
}

EMETH RandomWinner(edata *E) { return (int)RandInt(E->NumCands); }

EMETH RandomBallot(edata *E)
{ /*honest top choice of a random voter is elected. Strategyproof.*/
    int winner;
    winner = ArgMaxRealArr(E->NumCands, E->PerceivedUtility + RandInt(E->NumVoters) * E->NumCands, (int *)RandCandPerm);
    return winner;
}

EMETH Hay(edata *E /*Strategyproof. Prob of election proportional to sum of sqrts of [normalized] utilities*/
)
{ /* side effects: UtilityRootSum[] */
    uint x;
    int i, j, winner;
    real minu, sumrts, t;
    ZeroRealArray(E->NumCands, UtilityRootSum);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        minu = HUGE;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            if (minu > E->Utility[x + j])
                minu = E->Utility[x + j];
        }
        sumrts = 0.0;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            sumrts += sqrt(E->Utility[x + j] - minu);
        }
        if (sumrts > 0.0)
        {
            for (j = E->NumCands - 1; j >= 0; j--)
            {
                UtilityRootSum[j] += sqrt(E->Utility[x + j] - minu) / sumrts;
            }
        }
    }
    sumrts = 0.0;
    for (j = E->NumCands - 1; j >= 0; j--)
    {
        sumrts += UtilityRootSum[j];
    }
    t = Rand01() * sumrts;
    sumrts = 0.0;
    winner = -1;
    for (j = E->NumCands - 1; j >= 0; j--)
    {
        sumrts += UtilityRootSum[j];
        if (t < sumrts)
        {
            winner = j;
            break;
        }
    }
    assert(winner >= 0);
    return winner;
}

EMETH Plurality(edata *E /* canddt with most top-rank votes wins */
)
{ /* side effects: PlurVoteCount[], PlurWinner */
    int i;
    ZeroIntArray(E->NumCands, (int *)PlurVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        PlurVoteCount[E->TopDownPrefs[i * E->NumCands + 0]]++;
    }
    PlurWinner = ArgMaxIntArr(E->NumCands, (int *)PlurVoteCount, (int *)RandCandPerm);
    return PlurWinner;
}

EMETH AntiPlurality(edata *E /* canddt with fewest bottom-rank votes wins */
)
{ /* side effects: AntiPlurVoteCount[], AntiPlurWinner */
    int i;
    ZeroIntArray(E->NumCands, (int *)AntiPlurVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        AntiPlurVoteCount[E->TopDownPrefs[i * E->NumCands + E->NumCands - 1]]++;
    }
    AntiPlurWinner = ArgMinIntArr(E->NumCands, (int *)AntiPlurVoteCount, (int *)RandCandPerm);
    return AntiPlurWinner;
}

/* Plurality needs to have already been run before running Dabagh.  */
EMETH Dabagh(edata *E /* canddt with greatest Dabagh = 2*#top-rank-votes + 1*#second-rank-votes, wins */
)
{ /* side effects: DabaghVoteCount[] */
    int i, winner;
    CopyIntArray(E->NumCands, (int *)PlurVoteCount, (int *)DabaghVoteCount);
    ScaleIntVec(E->NumCands, (int *)DabaghVoteCount, 2);
    for (i = 0; i < (int)E->NumVoters; i++)
    { /*add 2nd-pref votes with weight=1:*/
        DabaghVoteCount[E->TopDownPrefs[i * E->NumCands + 1]]++;
    }
    winner = ArgMaxIntArr(E->NumCands, (int *)DabaghVoteCount, (int *)RandCandPerm);
    return winner;
}

/* Plurality needs to have already been run before running VtForAgainst.  */
EMETH VtForAgainst(edata *E /* canddt with greatest score = #votesFor - #votesAgainst,  wins */
)
{ /* side effects: VFAVoteCount[] */
    int i, winner, last;
    last = E->NumCands - 1;
    CopyIntArray(E->NumCands, (int *)PlurVoteCount, VFAVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        VFAVoteCount[(int)E->TopDownPrefs[i * E->NumCands + last]]--;
    }
    winner = ArgMaxIntArr(E->NumCands, VFAVoteCount, (int *)RandCandPerm);
    return winner;
}

EMETH Top2Runoff(edata *E /* Top2Runoff=top-2-runoff, 2nd round has fully-honest voting */
)
{ /* side effects: PSecond */
    int i;
    uint offset, pwct = 0, wct = 0;
    PSecond = -1;
    if (PlurWinner < 0)
        Plurality(E);
    PSecond = Arg2MaxUIntArr(E->NumCands, PlurVoteCount, (int *)RandCandPerm, PlurWinner);
    assert(PSecond >= 0);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        offset = i * E->NumCands;
        if (E->PerceivedUtility[offset + PlurWinner] > E->PerceivedUtility[offset + PSecond])
        {
            pwct++;
        }
        else if (E->PerceivedUtility[offset + PlurWinner] < E->PerceivedUtility[offset + PSecond])
        {
            wct++;
        }
    }
    if (pwct > wct || (pwct == wct && RandBool()))
        return (PlurWinner);
    return (PSecond);
}

EMETH VenzkeDisqPlur(edata *E /* Plurality winner wins unless over 50% vote him bottom; then plurality 2nd-winner wins. */
)
{ /* side effects: VFAVoteCount[] */
    if (PlurWinner < 0)
        Plurality(E);
    if (AntiPlurWinner < 0)
        AntiPlurality(E);
    if (2 * AntiPlurVoteCount[PlurWinner] > E->NumVoters)
    {
        if (PSecond < 0)
            Top2Runoff(E);
        return PSecond;
    }
    return PlurWinner;
}

EMETH PlurIR(edata *E /* PlurIR=plur+immediate runoff (give ranking as vote) */
)
{
    int i;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (PSecond < 0)
        Top2Runoff(E);
    if (PlurWinner < 0)
        Plurality(E);
    if (PSecond < 0)
        Top2Runoff(E);
    assert(PSecond >= 0);
    i = E->MarginsMatrix[PlurWinner * E->NumCands + PSecond];
    if (i > 0)
        return (PlurWinner);
    if (i < 0 || RandBool())
        return (PSecond);
    return (PlurWinner);
}

EMETH Borda(edata *E /* Borda: weighted positional with weights N-1, N-2, ..., 0  if N-canddts */
)
{ /* side effects: BordaVoteCount[], BordaWinner */
    int i, j, t;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        t = 0;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            t += E->MarginsMatrix[i * E->NumCands + j];
        }
        BordaVoteCount[i] = t;
    }
    BordaWinner = ArgMaxIntArr(E->NumCands, (int *)BordaVoteCount, (int *)RandCandPerm);
    return BordaWinner;
}

EMETH Black(edata *E /* Condorcet winner if exists, else use Borda */
)
{
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (CondorcetWinner >= 0)
        return CondorcetWinner;
    if (BordaWinner < 0)
        Borda(E);
    return BordaWinner;
}

EMETH RandomPair(edata *E)
{ /*pairwise honest-util winner among 2 random candidates is elected*/
    int x, y;
    x = (int)RandInt(E->NumCands);
    y = (int)RandInt(E->NumCands);
    if (UtilitySum[x] > UtilitySum[y])
        return x;
    if (UtilitySum[x] < UtilitySum[y])
        return y;
    if (RandBool())
        return (y);
    return (x);
}

EMETH NansonBaldwin(edata *E /* repeatedly eliminate Borda loser */
)
{ /* side effects: NansonVoteCount[], Eliminated[] */
    int i, BordaLoser, rnd, minc, r;
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return CondorcetWinner;
    if (BordaWinner < 0)
        Borda(E);
    FillBoolArray(E->NumCands, Eliminated, FALSE);
    CopyIntArray(E->NumCands, (int *)BordaVoteCount, NansonVoteCount);
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (rnd = 1; rnd < (int)E->NumCands; rnd++)
    {
        BordaLoser = -1;
        minc = BIGINT;
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            r = RandCandPerm[i];
            if (!Eliminated[r] && NansonVoteCount[r] < minc)
            {
                minc = NansonVoteCount[r];
                BordaLoser = r;
            }
        }
        assert(BordaLoser >= 0);
        Eliminated[BordaLoser] = TRUE;
        for (i = E->NumCands - 1; i >= 0; i--)
            if (!Eliminated[i])
            {
                NansonVoteCount[i] -= E->MarginsMatrix[i * E->NumCands + BordaLoser];
            }
    } /* end of for(rnd) */
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find non-eliminated candidate... */
        if (!Eliminated[i])
        {
            return i; /*NansonBaldwin winner*/
        }
    }
    return (-1); /*error*/
}

/***
 * Rouse is like Nanson-Baldwin but with an extra level of recursion: it successively
 * pseudo-eliminates the candidate with the highest Borda score until one is left, then it
 * genuinely-eliminates that one from the
 * original list; this step is repeated until a single candidate is left.
 * Slow: O(N^4) steps to find winner for N candidates from pairwise matrix.
 ******/
EMETH Rouse(edata *E /*like Nanson-Baldwin but with an extra level of recursion BUGGY*/
)
{ /* side effects: Rmark[], rRmark[] */
    int i, j, k, m, r, highestb, bordsum, maxb, winner;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    FillBoolArray(E->NumCands, rRmark, TRUE); /* nobody eliminated initially */
    for (m = E->NumCands; m > 1; m--)
    { /* NumCands-1 elimination-rounds */
        for (k = 1; k < m; k++)
        { /* m-1 pseudo-elimination rounds */
            maxb = -BIGINT;
            highestb = -1;
            FillBoolArray(E->NumCands, Rmark, TRUE); /* nobody pseudo-eliminated initially */
            RandomlyPermute(E->NumCands, RandCandPerm);
            for (i = E->NumCands - 1; i >= 0; i--)
            {
                r = RandCandPerm[i];
                if (rRmark[r] && Rmark[r])
                {
                    bordsum = 0;
                    for (j = E->NumCands - 1; j >= 0; j--)
                    {
                        if (Rmark[j] && rRmark[j])
                            bordsum += E->MarginsMatrix[r * E->NumCands + j];
                    }
                    if (maxb < bordsum)
                    {
                        maxb = bordsum;
                        highestb = r;
                    }
                }
            }
            assert(highestb >= 0);
            assert(rRmark[highestb]);
            assert(Rmark[highestb]);
            Rmark[highestb] = FALSE; /* pseudo-eliminate borda-winner */
        }
        /* Find the non-psu-eliminated canddt i: */
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            if (Rmark[i] && rRmark[i])
            {
                break;
            }
        }
        assert(i >= 0);
        assert(i < (int)E->NumCands);
        rRmark[i] = FALSE; /* (genuinely) eliminate it */
    }
    winner = -1;
    for (k = 0; k < (int)E->NumCands; k++)
    {
        if (rRmark[k])
        {
            winner = k;
            break;
        }
    }
    assert(winner == 0 || !rRmark[0]);
    /***???
  if(winner != CondorcetWinner && CondorcetWinner>=0){ *???*
    printf("Rouse aint Condorcet (RW=%d CW=%d):\n", winner, CondorcetWinner);
    PrintEdata(stdout, E);
  }***/
    return winner;
}

EMETH IterCopeland(edata *E /*iterate Copeland on tied-winner set from previous iter until fixpt*/
)
{ /*Idea of this is due to Alex Small. side effects: summ[] */
    int i, r, j, z, maxc, winner, tiect, oldtiect, x, mxs;
    assert(E->NumCands >= 2);
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return (CondorcetWinner);
    winner = -1;
    oldtiect = -1;
    ZeroIntArray(E->NumCands, summ);
    RandomlyPermute(E->NumCands, RandCandPerm);
    mxs = 0;
    for (z = E->NumCands; z > 0; z--)
    {
        tiect = 0;
        for (i = E->NumCands - 1; i >= 0; i--)
            if (summ[i] < mxs)
            {
                summ[i] = -BIGINT;
            }
        maxc = -BIGINT;
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            r = RandCandPerm[i];
            if (summ[r] >= mxs)
            {
                x = r * E->NumCands;
                for (j = E->NumCands - 1; j >= 0; j--)
                {
                    if (j != r && summ[j] >= mxs)
                    {
                        summ[r] += 1 + SignInt(E->MarginsMatrix[x + j]);
                    }
                }
                if (summ[r] >= maxc)
                {
                    if (summ[r] == maxc)
                    {
                        tiect++;
                    }
                    else
                    {
                        maxc = summ[r];
                        winner = r;
                        tiect = 1;
                    }
                }
            }
        }
        assert(tiect >= 1);
        if (tiect <= 1 || tiect == oldtiect)
            return (winner);
        oldtiect = tiect;
    }
    return (-1);
}

EMETH Nauru(edata *E /* weighted positional with weights 1, 1/2, 1/3, 1/4,... */
)
{ /* side effects: NauruVoteCount[] */
    int i, j, x, winner;
    ZeroIntArray(E->NumCands, (int *)NauruVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            NauruVoteCount[j] += NauruWt[E->CandRankings[x + j]];
        }
    }
    winner = ArgMaxUIntArr(E->NumCands, NauruVoteCount, (int *)RandCandPerm);
    return (winner);
}

EMETH HeismanTrophy(edata *E /* Heisman: weighted positional with weights 3, 2, 1, 0,...,0 */
)
{ /* side effects: HeismanVoteCount[] */
    int i, j, x, winner;
    ZeroIntArray(E->NumCands, (int *)HeismanVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = MinInt(E->NumCands - 1, 2); j >= 0; j--)
        {
            HeismanVoteCount[E->TopDownPrefs[x + j]] += 3 - j;
        }
    }
    winner = ArgMaxUIntArr(E->NumCands, HeismanVoteCount, (int *)RandCandPerm);
    return (winner);
}

EMETH BaseballMVP(edata *E /* weighted positional with weights 14,9,8,7,6,5,4,3,2,1,0,...,0 */
)
{ /* side effects: BaseballVoteCount[] */
    int i, j, x, winner;
    ZeroIntArray(E->NumCands, (int *)BaseballVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = MinInt(E->NumCands - 1, 9); j >= 0; j--)
        {
            BaseballVoteCount[E->TopDownPrefs[x + j]] += BaseballWt[j];
        }
    }
    winner = ArgMaxUIntArr(E->NumCands, BaseballVoteCount, (int *)RandCandPerm);
    return (winner);
}

EMETH CondorcetLR(edata *E /* candidate with least sum-of-pairwise-defeat-margins wins */
)
{ /* side effects:, SumOfDefeatMargins[]  */
    int i, j, t, winner;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return (CondorcetWinner);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        t = 0;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            t += PosInt(E->MarginsMatrix[j * E->NumCands + i]);
        }
        SumOfDefeatMargins[i] = t;
    }
    winner = ArgMinIntArr(E->NumCands, (int *)SumOfDefeatMargins, (int *)RandCandPerm);
    return winner;
}

EMETH Sinkhorn(edata *E /* candidate with max Sinkhorn rating (from all-positive DefeatsMatrix+1) */
)
{ /* side effects SinkRat[], SinkRow[], SinkCol[], SinkMat[] */
    int j, k, winner;
    real t, maxsum, minsum, sum, maxminRatio;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    FillRealArray(E->NumCands, SinkRow, 1.0);
    FillRealArray(E->NumCands, SinkCol, 1.0);
    do
    {
        for (k = 0; k < (int)E->NumCands; k++)
        {
            for (j = E->NumCands - 1; j >= 0; j--)
            {
                SinkMat[k * E->NumCands + j] =
                    SinkRow[k] * SinkCol[j] * (E->DefeatsMatrix[k * E->NumCands + j] + 1.0);
            }
        }
        maxsum = -HUGE;
        minsum = HUGE;
        for (k = 0; k < (int)E->NumCands; k++)
        {
            sum = 0.0;
            for (j = E->NumCands - 1; j >= 0; j--)
            {
                sum += SinkMat[k * E->NumCands + j];
            }
            if (minsum > sum)
                minsum = sum;
            if (maxsum < sum)
                maxsum = sum;
            SinkRow[k] /= sum;
        }
        maxminRatio = maxsum / minsum;
        maxsum = -HUGE;
        minsum = HUGE;
        for (k = 0; k < (int)E->NumCands; k++)
        {
            sum = 0.0;
            for (j = E->NumCands - 1; j >= 0; j--)
            {
                sum += SinkMat[j * E->NumCands + k];
            }
            if (minsum > sum)
                minsum = sum;
            if (maxsum < sum)
                maxsum = sum;
            SinkCol[k] /= sum;
        }
        t = maxsum / minsum;
        if (maxminRatio < t)
            maxminRatio = t;
    }
    until(maxminRatio < 1.000003);
    for (j = E->NumCands - 1; j >= 0; j--)
    {
        SinkRat[j] = SinkCol[j] / SinkRow[j];
    }
    winner = ArgMaxRealArr(E->NumCands, SinkRat, (int *)RandCandPerm);
    return winner;
}

EMETH KeenerEig(edata *E /* winning canddt has max Frobenius eigenvector entry (of all-positive DefeatsMatrix+1) */
)
{ /* Side effects EigVec[], EigVec2[] */
    int j, k, winner;
    real t, sum, dist;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    FillRealArray(E->NumCands, EigVec, 1.0);
    FillRealArray(E->NumCands, EigVec2, 1.0);
    do
    {
        for (k = 0; k < (int)E->NumCands; k++)
        {
            t = 0;
            for (j = E->NumCands - 1; j >= 0; j--)
            {
                t += (E->DefeatsMatrix[k * E->NumCands + j] + 1.0) * EigVec[j];
            }
            EigVec2[k] = t;
        }
        sum = SumRealArray(E->NumCands, EigVec2);
        ScaleRealVec(E->NumCands, EigVec2, 1.0 / sum);
        dist = L1Distance(E->NumCands, EigVec, EigVec2);
        CopyRealArray(E->NumCands, EigVec2, EigVec);
    }
    until(dist < 0.00001);
    winner = ArgMaxRealArr(E->NumCands, EigVec, (int *)RandCandPerm);
    return winner;
}

EMETH SimpsonKramer(edata *E /* candidate with mildest worst-defeat wins */
)
{ /* Side effects: WorstDefeatMargin[] */
    int i, r, x, t, j, winner;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return (CondorcetWinner);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        t = 0;
        RandomlyPermute(E->NumCands, RandCandPerm);
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            r = RandCandPerm[j];
            x = E->MarginsMatrix[r * E->NumCands + i];
            if (x > t)
                t = x;
        }
        WorstDefeatMargin[i] = t;
    }
    winner = ArgMinIntArr(E->NumCands, WorstDefeatMargin, (int *)RandCandPerm);
    return winner;
}

EMETH RaynaudElim(edata *E /* repeatedly eliminate canddt who suffered the worst-margin-defeat */
)
{ /* side effects: Eliminated[], RayDefeatMargin[], RayBeater[] */
    int i, j, x, t, RayLoser, rnd, maxc, r, beater;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return CondorcetWinner;
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        t = -BIGINT;
        beater = -1;
        RandomlyPermute(E->NumCands, RandCandPerm);
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            r = RandCandPerm[j];
            x = E->MarginsMatrix[r * E->NumCands + i];
            if (x > t)
            {
                t = x;
                beater = r;
            }
        }
        assert(beater >= 0);
        RayDefeatMargin[i] = t; /*worst margin of defeat of i, nonpositive if undefeated */
        RayBeater[i] = beater;  /*who administered that beating*/
    }
    FillBoolArray(E->NumCands, Eliminated, FALSE);
    for (rnd = 1; rnd < (int)E->NumCands; rnd++)
    {
        RayLoser = -1;
        maxc = -BIGINT;
        RandomlyPermute(E->NumCands, RandCandPerm);
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            r = RandCandPerm[i];
            if (!Eliminated[r] && RayDefeatMargin[r] > maxc)
            {
                maxc = RayDefeatMargin[r];
                RayLoser = r;
            }
        }
        assert(RayLoser >= 0);
        if (maxc <= 0)
        {
            return RayLoser;
        } /*"loser" is undefeated*/
        Eliminated[RayLoser] = TRUE;
        for (i = E->NumCands - 1; i >= 0; i--)
            if (!Eliminated[i] && RayBeater[i] == RayLoser)
            {
                t = -BIGINT;
                beater = -1;
                RandomlyPermute(E->NumCands, RandCandPerm);
                for (j = E->NumCands - 1; j >= 0; j--)
                {
                    r = RandCandPerm[j];
                    if (!Eliminated[r])
                    {
                        x = E->MarginsMatrix[r * E->NumCands + i];
                        if (x > t)
                        {
                            t = x;
                            beater = r;
                        }
                    }
                }
                assert(beater >= 0);
                RayDefeatMargin[i] = t;
                RayBeater[i] = beater;
            }
    } /* end of for(rnd) */
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find non-eliminated candidate... */
        if (!Eliminated[i])
        {
            return i; /*Raynaud winner*/
        }
    }
    return (-1); /*error*/
}

EMETH ArrowRaynaud(edata *E /* repeatedly eliminate canddt with smallest {largest margin of victory, which is <=0 if never won} */
)
{ /* side effects: Eliminated[], ARchump[], ARVictMargin[].  ArrowRaynaud can eliminate a Condorcet Winner in round #1. */
    int i, j, x, t, ARLoser, rnd, minc, r, chump;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        t = -BIGINT;
        chump = -1;
        RandomlyPermute(E->NumCands, RandCandPerm);
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            r = RandCandPerm[j];
            x = E->MarginsMatrix[i * E->NumCands + r];
            if (x > t)
            {
                t = x;
                chump = r;
            }
        }
        assert(chump >= 0);
        ARVictMargin[i] = t; /*largest margin of victory of i, nonpositive if never won*/
        ARchump[i] = chump;  /*who suffered that beating*/
    }
    FillBoolArray(E->NumCands, Eliminated, FALSE);
    for (rnd = 1; rnd < (int)E->NumCands; rnd++)
    {
        ARLoser = -1;
        minc = BIGINT;
        RandomlyPermute(E->NumCands, RandCandPerm);
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            r = RandCandPerm[i];
            if (!Eliminated[r] && ARVictMargin[r] < minc)
            {
                minc = ARVictMargin[r];
                ARLoser = r;
            }
        }
        assert(ARLoser >= 0);
        Eliminated[ARLoser] = TRUE;
        for (i = E->NumCands - 1; i >= 0; i--)
            if (!Eliminated[i] && ARchump[i] == ARLoser)
            {
                t = -BIGINT;
                chump = -1;
                RandomlyPermute(E->NumCands, RandCandPerm);
                for (j = E->NumCands - 1; j >= 0; j--)
                {
                    r = RandCandPerm[j];
                    if (!Eliminated[r])
                    {
                        x = E->MarginsMatrix[i * E->NumCands + r];
                        if (x > t)
                        {
                            t = x;
                            chump = r;
                        }
                    }
                }
                assert(chump >= 0);
                ARVictMargin[i] = t;
                ARchump[i] = chump;
            }
    } /* end of for(rnd) */
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find non-eliminated candidate... */
        if (!Eliminated[i])
        {
            return i; /*ArrowRaynaud winner*/
        }
    }
    return (-1); /*error*/
}

/* O(N^3) algorithm, but it is known how to speed it up to O(N^2) */
EMETH SchulzeBeatpaths(edata *E /* winner = X so BeatPathStrength over rivals Y exceeds strength from Y */
)
{ /* Side effects: BeatPathStrength[] */
    int i, j, k, minc, winner;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        for (j = E->NumCands - 1; j >= 0; j--)
            if (i != j)
            {
                BeatPathStrength[i * E->NumCands + j] = E->MarginsMatrix[i * E->NumCands + j];
            }
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        for (j = E->NumCands - 1; j >= 0; j--)
            if (i != j)
            {
                for (k = 0; k < (int)E->NumCands; k++)
                    if (k != j && k != i)
                    {
                        minc = BeatPathStrength[j * E->NumCands + i];
                        if (BeatPathStrength[i * E->NumCands + k] < minc)
                            minc = BeatPathStrength[i * E->NumCands + k];
                        if (BeatPathStrength[j * E->NumCands + k] < minc)
                            BeatPathStrength[j * E->NumCands + k] = minc;
                    }
            }
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        k = RandCandPerm[i];
        for (j = E->NumCands - 1; j >= 0; j--)
            if (k != j)
            {
                if (BeatPathStrength[j * E->NumCands + k] > BeatPathStrength[k * E->NumCands + j])
                {
                    goto KNOTSCHULZEWINNER;
                }
            }
        winner = k;
        return winner;
    KNOTSCHULZEWINNER:;
    }
    return (-1);
}

/* Note about Smith and Schwartz sets:
BeatPathStrength[k*E->NumCands+j] > 0   for all k in the "Smith Set" and j outside it.
BeatPathStrength[k*E->NumCands+j] >= 0  for all k in the "Schwartz Set" and j outside it.
*******/

void beatDFS(int x, int diff, bool Set[], int Mat[], int N)
{
    int i;
    for (i = N - 1; i >= 0; i--)
        if (i != x)
        {
            if (Mat[i * N + x] >= diff)
            {
                if (!Set[i])
                {
                    Set[i] = TRUE;
                    beatDFS(i, diff, Set, Mat, N);
                }
            }
        }
}

EMETH SmithSet(edata *E /* Smith set = smallest nonempty set of canddts that pairwise-beat all nonmembers */
)
{ /* side effects: SmithMembs[] */
    int i, r;
    FillBoolArray(E->NumCands, SmithMembs, FALSE);
    assert(CopeWinOnlyWinner >= 0);
    assert(CopeWinOnlyWinner < (int)E->NumCands);
    SmithMembs[CopeWinOnlyWinner] = TRUE;
    beatDFS(CopeWinOnlyWinner, 1, SmithMembs, E->MarginsMatrix, E->NumCands);
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        r = RandCandPerm[i];
        if (SmithMembs[r])
            return r; /*return random set member*/
    }
    return (-1);
}

EMETH SchwartzSet(edata *E /* Schwartz set = smallest nonempty set of canddts undefeated by nonmembers */
)
{ /* side effects: SchwartzMembs[] */
    int i, r;
    FillBoolArray(E->NumCands, SchwartzMembs, FALSE);
    assert(CopeWinOnlyWinner >= 0);
    assert(CopeWinOnlyWinner < (int)E->NumCands);
    SchwartzMembs[CopeWinOnlyWinner] = TRUE;
    beatDFS(CopeWinOnlyWinner, 0, SchwartzMembs, E->MarginsMatrix, E->NumCands);
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        r = RandCandPerm[i];
        if (SchwartzMembs[r])
            return r; /*return random set member*/
    }
    return (-1);
}

/* Uncovered set.  A "covers" B if the candidates A beats pairwise 
are a superset of those B beats pairwise.  "Landau's theorem":
All Copeland winners are members of the uncovered set.
Can do this an order of magnitude faster if MaxNumCands<sizeof(uint)*4 because set inclusion can
be tested in 1 step using wordwide operations??
****/
EMETH SlowUncoveredSet(edata *E /*A "covers" B if A beats a strict?? superset of those B beats.*/
)
{ /* side effects: UncoveredSt[], CoverMatrix[] */
    int A, B, C, i, r;
    bool cov;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (SchwartzWinner < 0)
        SchwartzSet(E);
    /*find cover relation:*/
    for (A = 0; A < (int)E->NumCands; A++)
    {
        for (B = 0; B < (int)E->NumCands; B++)
            if (B != A)
            {
                cov = TRUE;
                for (C = 0; C < (int)E->NumCands; C++)
                {
                    if (E->MarginsMatrix[A * E->NumCands + C] <= 0 &&
                        E->MarginsMatrix[B * E->NumCands + C] > 0)
                    {
                        cov = FALSE;
                        break;
                    }
                }
                CoverMatrix[A * E->NumCands + B] = cov;
            }
        UncoveredSt[A] = TRUE; /*initialization*/
    }
    for (A = 0; A < (int)E->NumCands; A++)
    {
        for (B = 0; B < A; B++)
            if (B != A)
            {
                if (CoverMatrix[A * E->NumCands + B] && CoverMatrix[B * E->NumCands + A])
                {
                    /*enforce strict superset; otherwise coverage both ways would be possible:*/
                    CoverMatrix[A * E->NumCands + B] = FALSE;
                    CoverMatrix[B * E->NumCands + A] = FALSE;
                }
            }
    }
    /*find UncoveredSt:*/
    for (A = 0; A < (int)E->NumCands; A++)
    {
        for (B = 0; B < (int)E->NumCands; B++)
            if (B != A)
            {
                if (CoverMatrix[B * E->NumCands + A])
                {
                    UncoveredSt[A] = FALSE;
                    break;
                }
            }
    }
    /*select random uncovered winner:*/
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        if (!(UncoveredSt[i] ? SchwartzMembs[i] : TRUE))
        {
            printf("bozo!\n");
            printf("%d %d %d %d %d %d %d %d %d\n",
                   E->MarginsMatrix[0 * E->NumCands + 0],
                   E->MarginsMatrix[0 * E->NumCands + 1],
                   E->MarginsMatrix[0 * E->NumCands + 2],
                   E->MarginsMatrix[1 * E->NumCands + 0],
                   E->MarginsMatrix[1 * E->NumCands + 1],
                   E->MarginsMatrix[1 * E->NumCands + 2],
                   E->MarginsMatrix[2 * E->NumCands + 0],
                   E->MarginsMatrix[2 * E->NumCands + 1],
                   E->MarginsMatrix[2 * E->NumCands + 2]);
            printf("CopeWinOnlyWinner=%d\n", CopeWinOnlyWinner);
            printf("Sc=%d%d%d\n", SchwartzMembs[0], SchwartzMembs[1], SchwartzMembs[2]);
            printf("Un=%d%d%d\n", UncoveredSt[0], UncoveredSt[1], UncoveredSt[2]);
        }
        assert(UncoveredSt[i] ? SchwartzMembs[i] : TRUE);
        r = RandCandPerm[i];
        if (UncoveredSt[r])
        {
            RandomUncoveredMemb = r;
            return r;
        }
    }
    printf("yikes!\n");
    printf("%d %d %d %d\n",
           E->MarginsMatrix[0 * E->NumCands + 0],
           E->MarginsMatrix[1 * E->NumCands + 0],
           E->MarginsMatrix[0 * E->NumCands + 1],
           E->MarginsMatrix[1 * E->NumCands + 1]);
    return (-1);
}

/* Uncovered set.  A "covers" B if the candidates A beats pairwise 
are a superset of those B beats pairwise.  "Landau's theorem":
All Copeland winners are members of the uncovered set.
Now an order of magnitude faster assuming MaxNumCands<sizeof(uint)*4 because set inclusion can
be tested in 1 step using wordwide operations.
****/
EMETH UncoveredSet(edata *E /*A "covers" B if A beats a strict superset of those B beats.*/
)
{ /* side effects: UncoveredSt[], CoverMatrix[] */
    int A, B, i, r;
    if (E->NumCands > 4 * sizeof(uint))
    {
        printf("UncoveredSet: too many candidates %d to use machine words(%d) to represent sets\n",
               E->NumCands,
               (int)(4 * sizeof(uint)));
        printf("You could rewrite the code to use uint64s to try allow up to 64 canddts\n");
        exit(EXIT_FAILURE);
    }
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (SchwartzWinner < 0)
        SchwartzSet(E);
    /*find cover relation:*/
    for (A = 0; A < (int)E->NumCands; A++)
    {
        for (B = 0; B < (int)E->NumCands; B++)
            if (B != A)
            {
                CoverMatrix[A * E->NumCands + B] = StrictSuperset(IBeat[A], IBeat[B]);
            }
        UncoveredSt[A] = TRUE; /*initialization*/
    }
    /*find UncoveredSt:*/
    for (A = 0; A < (int)E->NumCands; A++)
    {
        for (B = 0; B < (int)E->NumCands; B++)
            if (B != A)
            {
                if (CoverMatrix[B * E->NumCands + A])
                {
                    UncoveredSt[A] = FALSE;
                    break;
                }
            }
    }
    /*select random uncovered winner:*/
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        if (!(UncoveredSt[i] ? SchwartzMembs[i] : TRUE))
        {
            printf("bozo! i=%d NumCands=%d\n", i, E->NumCands);
            printf("%d %d %d; %d %d %d; %d %d %d\n",
                   E->MarginsMatrix[0 * E->NumCands + 0],
                   E->MarginsMatrix[0 * E->NumCands + 1],
                   E->MarginsMatrix[0 * E->NumCands + 2],
                   E->MarginsMatrix[1 * E->NumCands + 0],
                   E->MarginsMatrix[1 * E->NumCands + 1],
                   E->MarginsMatrix[1 * E->NumCands + 2],
                   E->MarginsMatrix[2 * E->NumCands + 0],
                   E->MarginsMatrix[2 * E->NumCands + 1],
                   E->MarginsMatrix[2 * E->NumCands + 2]);
            printf("CopeWinOnlyWinner=%d\n", CopeWinOnlyWinner);
            printf("Sc=%d%d%d\n", SchwartzMembs[0], SchwartzMembs[1], SchwartzMembs[2]);
            printf("Un=%d%d%d\n", UncoveredSt[0], UncoveredSt[1], UncoveredSt[2]);
        }
        assert(UncoveredSt[i] ? SchwartzMembs[i] : TRUE);
        r = RandCandPerm[i];
        if (UncoveredSt[r])
        {
            RandomUncoveredMemb = r;
            return r;
        }
    }
    printf("yikes!\n");
    printf("%d %d %d %d\n",
           E->MarginsMatrix[0 * E->NumCands + 0],
           E->MarginsMatrix[1 * E->NumCands + 0],
           E->MarginsMatrix[0 * E->NumCands + 1],
           E->MarginsMatrix[1 * E->NumCands + 1]);
    return (-1);
}

EMETH Bucklin(edata *E /* canddt with over half the voters ranking him in top-k wins (for k minimal) */
)
{ /* side effects: RdVoteCount[] */
    int rnd, i, best, winner;
    winner = -1;
    ZeroIntArray(E->NumCands, RdVoteCount);
    for (rnd = 0; rnd < (int)E->NumCands; rnd++)
    {
        for (i = 0; i < (int)E->NumVoters; i++)
        {
            RdVoteCount[E->TopDownPrefs[i * E->NumCands + rnd]]++;
        }
        winner = ArgMaxIntArr(E->NumCands, RdVoteCount, (int *)RandCandPerm);
        best = RdVoteCount[winner];
        if (best * 2 > (int)E->NumVoters)
            break;
    }
    return winner;
}

/******
James Green-Armytage:  Definition of the cardinal-weighted pairwise comparison method
I. Voters rate the candidates, e.g. on a scale from 0 to 100. 
Equal ratings are allowed.   There then is an implied ranking.
II. Tally
1. Determine the direction of the pairwise defeats by using the rankings for a standard pairwise 
comparison tally.
2. Determine the strength of the pairwise defeats by finding the weighted magnitude as follows. 
Suppose candidate A pairwise beats B, and we want to know the strength of the defeat. 
For each voter who ranks A over B, and only for voters who rank A over B, 
subtract their rating of B from their rating of A, to get the rating differential. 
The sum of these individual winning rating differentials is the total weighted magnitude of the defeat. 
(Note that, because voters who rank B over A do not contribute to the weighted magnitude of the A>B defeat, 
it cannot be negative.)
3. Now that the direction of the pairwise defeats have been determined (in step 1) and 
the strength of the defeats have been determined (in step 2), you can choose from a 
variety of Condorcet completion methods to determine the winner. 
I recommend the ranked pairs, beatpath, and river methods.
WDS: IEVS will use beatpaths.  What is good voter strategy in this method?
*********/

EMETH ArmytagePCSchulze(edata *E /*Armytage pairwise comparison based on Schulze*/
)
{ /* Side effects: ArmyBPS[] */
    int i, j, k, winner;
    real minc;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        for (j = E->NumCands - 1; j >= 0; j--)
            if (i != j)
            {
                ArmyBPS[i * E->NumCands + j] = E->MargArmy[i * E->NumCands + j];
            }
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        for (j = E->NumCands - 1; j >= 0; j--)
            if (i != j)
            {
                for (k = 0; k < (int)E->NumCands; k++)
                    if (k != j && k != i)
                    {
                        minc = ArmyBPS[j * E->NumCands + i];
                        if (ArmyBPS[i * E->NumCands + k] < minc)
                            minc = ArmyBPS[i * E->NumCands + k];
                        if (ArmyBPS[j * E->NumCands + k] < minc)
                            ArmyBPS[j * E->NumCands + k] = minc;
                    }
            }
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        k = RandCandPerm[i];
        for (j = E->NumCands - 1; j >= 0; j--)
            if (k != j)
            {
                if (ArmyBPS[j * E->NumCands + k] > ArmyBPS[k * E->NumCands + j])
                {
                    goto KNOTSW;
                }
            }
        winner = k;
        return winner;
    KNOTSW:;
    }
    return (-1);
}

EMETH Copeland(edata *E /* canddt with largest number of pairwise-wins elected (tie counts as win/2) BUGGY */
)
{ /* side effects: CopelandWinner, CopeScore[] */
    int i;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (CWSPEEDUP && CondorcetWinner >= 0)
    {
        CopelandWinner = CondorcetWinner;
        return CopelandWinner;
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        CopeScore[i] = 2 * WinCount[i] + DrawCt[i];
    }
    CopelandWinner = ArgMaxIntArr(E->NumCands, CopeScore, (int *)RandCandPerm);
    /* Currently just break ties randomly, return random highest-scorer */
    return CopelandWinner;
}

EMETH SimmonsCond(edata *E /* winner = X with least sum of top-rank-votes for rivals pairwise-beating X */
)
{ /* side effects: SimmVotesAgainst[] */
    int i, j, t, winner;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (PlurWinner < 0)
        Plurality(E);
    if (SmithWinner < 0)
        SmithSet(E);
    if (SchwartzWinner < 0)
        SchwartzSet(E);
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return (CondorcetWinner);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        t = 0;
        for (j = E->NumCands - 1; j >= 0; j--)
            if (E->MarginsMatrix[j * E->NumCands + i] > 0)
            { /* j pairwise-beats i */
                t += 3 * PlurVoteCount[j] + (SchwartzMembs[j] ? 1 : 0) + (SmithMembs[j] ? 1 : 0);
                /*Here I am adding 1/3 of a vote if in SmithSet, ditto SchwartzSet, to break Simmons ties*/
            }
        SimmVotesAgainst[i] = t;
    }
    winner = ArgMinIntArr(E->NumCands, SimmVotesAgainst, (int *)RandCandPerm);
    return winner;
}

/*******
 * The following IRV (instant runoff voting) algorithm has somewhat long code, but
 * by using linked lists achieves fast (very sublinear) runtime.  
 * If SmithIRVwinner<0 then It also computes SmithIRVwinner as a side effect.
 * This code can also be used to compute the winner in "Top3" (bastardized) IRV where
 * voters only allowed to indicate their top 3 choices in order..
 * For normal IRV (or SmithIRV) set IRVTopLim=BIGINT before run.
 * For Top3-IRV set IRVTopLim=3 (or any other integer N for TopN-IRV). ***/
EMETH IRV(edata *E /* instant runoff; repeatedly eliminate plurality loser */
)
{ /* side effects: Eliminated[], IFav[], RdVoteCount[], FavListNext[], HeadFav[], LossCount[], SmithIRVwinner, IRVwinner  */
    int Iround, i, RdLoser, NextI, j, t, x, minc, r, stillthere, winner;
    assert(E->NumCands <= MaxNumCands);
    if (SmithIRVwinner < 0 && IRVTopLim == BIGINT && CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        Eliminated[i] = FALSE;
        HeadFav[i] = -1; /*HeadFav[i] will be the first voter whose current favorite is i*/
        if (SmithIRVwinner < 0 && IRVTopLim == BIGINT)
        {
            t = 0;
            for (j = E->NumCands - 1; j >= 0; j--)
                if (E->MarginsMatrix[j * E->NumCands + i] > 0)
                {
                    t++;
                }
            LossCount[i] = t;
        }
    } /*end for(i)*/
    ZeroIntArray(E->NumCands, RdVoteCount);
    ZeroIntArray(E->NumVoters, IFav);
    /* IFav[i] is the rank of the 1st noneliminated canddt in voter i's topdownpref list (initially 0) */
    FillIntArray(E->NumVoters, FavListNext, -1);
    /* FavListNext is "next" indices in linked list of voters with common current favorite; -1 terminated. */
    if (SmithIRVwinner < 0 && IRVTopLim == BIGINT)
    {
        SmithIRVwinner = CondorcetWinner;
    }
    /* compute vote totals for 1st round and set up forward-linked lists (-1 terminates each list): */
    for (i = E->NumVoters - 1; i >= 0; i--)
    {
        x = E->TopDownPrefs[i * E->NumCands + IFav[i]]; /* the initial favorite of voter i */
        assert(x >= 0);
        assert(x < (int)E->NumCands);
        RdVoteCount[x]++;
        FavListNext[i] = HeadFav[x];
        HeadFav[x] = i;
    }
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (Iround = 1; Iround < (int)E->NumCands; Iround++)
    { /*perform IRV rounds*/
        RdLoser = -1;
        minc = BIGINT;
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            r = RandCandPerm[i];
            if (!Eliminated[r] && RdVoteCount[r] < minc)
            {
                minc = RdVoteCount[r];
                RdLoser = r;
            }
        }
        assert(RdLoser >= 0);
        assert(RdLoser < (int)E->NumCands);
        Eliminated[RdLoser] = TRUE; /* eliminate RdLoser */
        if (IRVTopLim == BIGINT && SmithIRVwinner < 0)
        {
            for (j = E->NumCands - 1; j >= 0; j--)
                if (!Eliminated[j])
                { /* update LossCount[j] */
                    t = E->MarginsMatrix[RdLoser * E->NumCands + j];
                    if (t > 0)
                    {
                        LossCount[j]--;
                    }
                    if (LossCount[j] <= 0)
                    {
                        SmithIRVwinner = j;
                        break;
                    }
                }
        }
        for (i = HeadFav[RdLoser]; i >= 0; i = NextI)
        { /*Go thru linked list of voters with favorite=RdLoser, adjust:*/
            j = i * E->NumCands;
            NextI = FavListNext[i];
            assert(IFav[i] >= 0);
            assert(IFav[i] < (int)E->NumCands);
            assert(E->TopDownPrefs[j + IFav[i]] == RdLoser);
            do
            {
                IFav[i]++;
                x = E->TopDownPrefs[j + IFav[i]];
            } while (Eliminated[x]);
            /* x is new favorite of voter i (or ran out of favorites) */
            assert(IFav[i] < (int)E->NumCands);
            assert(x >= 0);
            assert(x < (int)E->NumCands);
            /* update favorite-list: */
            FavListNext[i] = HeadFav[x];
            HeadFav[x] = i;
            /* update vote count totals: */
            if (IFav[i] < IRVTopLim)
            {
                RdVoteCount[x]++;
            }
        } /*end for(i)*/
    }     /* end of for(Iround) */
    stillthere = 0;
    if (IRVTopLim >= (int)E->NumCands)
        IRVwinner = -1;
    winner = -1;
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find non-eliminated candidate... */
        if (!Eliminated[i])
        {
            winner = i;
            stillthere++;
        }
    }
    if (IRVTopLim >= (int)E->NumCands)
        IRVwinner = winner;
    assert(stillthere == 1);
    return (winner);
}

EMETH SmithIRV(edata *E /*  Eliminate plurality loser until unbeaten candidate exists. */
)
{ /* must be run after IRV. */
    if (IRVwinner < 0)
    {
        SmithIRVwinner = -1;
        IRV(E);
    }
    return SmithIRVwinner;
}

EMETH Top3IRV(edata *E /* Top-3-choices-only IRV */
)
{
    int w;
    IRVTopLim = 3;
    w = IRV(E);
    IRVTopLim = BIGINT;
    return w;
}

EMETH BTRIRV(edata *E /* Repeatedly eliminate either plur loser or 2nd-loser (whoever loses pairwise) */
)
{ /* side effects: Eliminated[], IFav[], RdVoteCount[], FavListNext[], HeadFav[], */
    int Iround, x, i, RdLoser, RdLoser2, NextI, j, minc, r;
    assert(E->NumCands <= MaxNumCands);
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return (CondorcetWinner);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        Eliminated[i] = FALSE;
        HeadFav[i] = -1;
    }
    ZeroIntArray(E->NumCands, RdVoteCount);
    ZeroIntArray(E->NumVoters, IFav);
    /* compute vote totals for 1st round and set up forward-linked lists (-1 terminates each list): */
    for (i = E->NumVoters - 1; i >= 0; i--)
    {
        assert(IFav[i] >= 0);
        assert(IFav[i] < (int)E->NumCands);
        x = E->TopDownPrefs[i * E->NumCands + IFav[i]]; /* the favorite of voter i */
        assert(x >= 0);
        assert(x < (int)E->NumCands);
        RdVoteCount[x]++;
        FavListNext[i] = HeadFav[x];
        HeadFav[x] = i;
    }
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (Iround = 1; Iround < (int)E->NumCands; Iround++)
    {
        RdLoser = -1;
        minc = BIGINT;
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            r = RandCandPerm[i];
            if (!Eliminated[r] && RdVoteCount[r] < minc)
            {
                minc = RdVoteCount[r];
                RdLoser = r;
            }
        }
        assert(RdLoser >= 0);
        RdLoser2 = -1;
        minc = BIGINT;
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            r = RandCandPerm[i];
            if (!Eliminated[r] && RdVoteCount[r] < minc && r != RdLoser)
            {
                minc = RdVoteCount[r];
                RdLoser2 = r;
            }
        }
        assert(RdLoser2 >= 0);
        if (E->MarginsMatrix[RdLoser * E->NumCands + RdLoser2] > 0)
            RdLoser = RdLoser2;
        Eliminated[RdLoser] = TRUE; /* eliminate RdLoser */
        for (i = HeadFav[RdLoser]; i >= 0; i = NextI)
        { /* Go thru list of voters with favorite=RdLoser, adjust: */
            j = i * E->NumCands;
            assert(E->TopDownPrefs[j + IFav[i]] == RdLoser);
            assert(IFav[i] >= 0);
            assert(IFav[i] < (int)E->NumCands);
            do
            {
                IFav[i]++;
                x = E->TopDownPrefs[j + IFav[i]];
            } while (Eliminated[x]);
            /* x is new favorite of voter i */
            assert(IFav[i] < (int)E->NumCands);
            NextI = FavListNext[i];
            /* update favorite-list: */
            FavListNext[i] = HeadFav[x];
            HeadFav[x] = i;
            /* update vote count totals: */
            assert(x >= 0);
            assert(x < (int)E->NumCands);
            RdVoteCount[x]++;
        }
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find the non-eliminated candidate... */
        if (!Eliminated[i])
        {
            return i; /*IRV winner*/
        }
    }
    return (-1); /*error*/
}

EMETH Coombs(edata *E /*repeatedly eliminate antiplurality loser (with most bottom-rank votes)*/
)
{ /*side effects: Eliminated[], IFav[], RdVoteCount[], FavListNext[], HeadFav[] */
    int Iround, i, RdLoser, NextI, j, x, maxc, r, stillthere;
    assert(E->NumCands <= MaxNumCands);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        Eliminated[i] = FALSE;
        HeadFav[i] = -1; /*HeadFav[i] will be the first voter whose current most-hated is i*/
    }
    ZeroIntArray(E->NumCands, RdVoteCount);
    assert(E->NumCands >= 2);
    assert(E->NumCands <= MaxNumVoters);
    FillIntArray(E->NumVoters, IFav, E->NumCands - 1);
    /* IFav[i] is the rank of the last noneliminated canddt in voter i's topdownpref list 
   * (initially NumCands-1) */
    FillIntArray(E->NumVoters, FavListNext, -1);
    /* FavListNext is "next" indices in linked list of voters with common current favorite; -1 terminated. */
    /* compute vote totals for 1st round and set up forward-linked lists (-1 terminates each list): */
    for (i = E->NumVoters - 1; i >= 0; i--)
    {
        assert(IFav[i] >= 0);
        assert(IFav[i] < (int)E->NumCands);
        x = E->TopDownPrefs[i * E->NumCands + IFav[i]]; /* the initial most-hated of voter i */
        assert(x >= 0);
        assert(x < (int)E->NumCands);
        RdVoteCount[x]++; /*antiplurality voting*/
        FavListNext[i] = HeadFav[x];
        HeadFav[x] = i;
    }
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (Iround = 1; Iround < (int)E->NumCands; Iround++)
    {
        RdLoser = -1;
        maxc = -BIGINT;
        stillthere = 0;
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            r = RandCandPerm[i];
            assert(r >= 0);
            assert(r < (int)E->NumCands);
            if (!Eliminated[r])
            {
                stillthere++;
                if (RdVoteCount[r] > maxc)
                {
                    maxc = RdVoteCount[r];
                    RdLoser = r;
                }
            }
        }
        assert(RdLoser >= 0);
        Eliminated[RdLoser] = TRUE; /* eliminate RdLoser */
        for (i = HeadFav[RdLoser]; i >= 0; i = NextI)
        { /*Go thru linked list of voters with favorite=RdLoser, adjust:*/
            j = i * E->NumCands;
            assert(IFav[i] >= 0);
            assert(IFav[i] <= (int)E->NumCands);
            assert(E->TopDownPrefs[j + IFav[i]] == RdLoser);
            do
            {
                IFav[i]--;
                x = E->TopDownPrefs[j + IFav[i]];
            } while (Eliminated[x]);
            /* x is new favorite of voter i */
            assert(IFav[i] >= 0);
            NextI = FavListNext[i];
            /* update favorite-list: */
            FavListNext[i] = HeadFav[x];
            HeadFav[x] = i;
            /* update vote count totals: */
            assert(x >= 0);
            assert(x < (int)E->NumCands);
            RdVoteCount[x]++;
        }
    } /* end of for(Iround) */
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find the non-eliminated candidate... */
        if (!Eliminated[i])
        {
            return i; /*Coombs winner*/
        }
    }
    return (-1); /*error*/
}

EMETH Approval(edata *E /* canddt with most-approvals wins */
)
{ /* side effects: ApprovalVoteCount[], ApprovalWinner */
    int i, j, x;
    ZeroIntArray(E->NumCands, (int *)ApprovalVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            ApprovalVoteCount[j] += E->Approve[x + j];
        }
    }
    RandomlyPermute(E->NumCands, RandCandPerm);
    ApprovalWinner = ArgMaxUIntArr(E->NumCands, (uint *)ApprovalVoteCount, (int *)RandCandPerm);
    return (ApprovalWinner);
}

EMETH App2Runoff(edata *E /*top-2-runoff, 1stRd=approval, 2nd round has fully-honest voting*/
)
{ /* side effects: ASecond */
    int i;
    uint offset, pwct = 0, wct = 0;
    ASecond = -1;
    if (ApprovalWinner < 0)
        Approval(E);
    ASecond = Arg2MaxUIntArr(E->NumCands, ApprovalVoteCount, (int *)RandCandPerm, ApprovalWinner);
    assert(ASecond >= 0);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        offset = i * E->NumCands;
        if (E->PerceivedUtility[offset + ApprovalWinner] > E->PerceivedUtility[offset + ASecond])
        {
            pwct++;
        }
        else if (E->PerceivedUtility[offset + ApprovalWinner] < E->PerceivedUtility[offset + ASecond])
        {
            wct++;
        }
    }
    if (pwct > wct || (pwct == wct && RandBool()))
        return (ApprovalWinner);
    return (ASecond);
}

EMETH HeitzigDFC(edata *E)
{ /*winner of honest runoff between ApprovalWinner and RandomBallot winner*/
    int i, Rwnr;
    uint offset, pwct = 0, wct = 0;
    Rwnr = ArgMaxRealArr(E->NumCands,
                         E->PerceivedUtility + RandInt(E->NumVoters) * E->NumCands, (int *)RandCandPerm);
    if (ApprovalWinner < 0)
        Approval(E);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        offset = i * E->NumCands;
        if (E->PerceivedUtility[offset + ApprovalWinner] > E->PerceivedUtility[offset + Rwnr])
        {
            pwct++;
        }
        else if (E->PerceivedUtility[offset + ApprovalWinner] < E->PerceivedUtility[offset + Rwnr])
        {
            wct++;
        }
    }
    if (pwct > wct || (pwct == wct && RandBool()))
        return (ApprovalWinner);
    return (Rwnr);
}

EMETH HeitzigLFC(edata *E)
{ /*random canddt who is not "strongly beat" wins; y strongly beats x if Approval(y)>Approval(x) and less than Approval(x)/2 voters prefer x>y.*/
    /*Side effects: Eliminated[] */
    int i, j;
    FillBoolArray(E->NumCands, Eliminated, FALSE);
    for (j = E->NumCands - 1; j >= 0; j--)
    {
        for (i = E->NumCands - 1; i >= 0; i--)
        {
            if (ApprovalVoteCount[j] > ApprovalVoteCount[i])
            {
                if (2 * E->DefeatsMatrix[i * E->NumCands + j] < (int)ApprovalVoteCount[i])
                {
                    /*Candidate i is "strongly beat"*/
                    Eliminated[i] = TRUE;
                }
            }
        }
    }
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find random non-eliminated candidate... */
        j = RandCandPerm[i];
        if (!Eliminated[j])
        {
            return j; /*winner*/
        }
    }
    return (-1); /*error*/
}

/***
Brian Olson's IRNR system:
1. get from each voter a range-style vote-vector, entries in range [-10, +10].
2. normalize so sum of squares equals 1.   (Or in variant, use other power than 2.)
3. sum the normalized vote vectors.
4. disqualify choice with lowest sum.
5. re-normalize (effectively redistributing voters' votes to their remaining choices).
6. back to step 3 until only 1 candidate remains.
7. It wins.
***/
real IRNRPOWER = 2.0;
EMETH IRNR(edata *E /*Brian Olson's voting method described above*/
)
{ /* side effects: Eliminated[], SumNormedRatings[]*/
    int rd, i, j, x, r, loser;
    real s, t, minc;

    FillBoolArray(E->NumCands, Eliminated, FALSE);
    for (rd = E->NumCands; rd > 1; rd--)
    {
        ZeroRealArray(E->NumCands, SumNormedRating);
        for (i = 0; i < (int)E->NumVoters; i++)
        {
            x = i * E->NumCands;
            s = 0.0;
            for (j = E->NumCands - 1; j >= 0; j--)
                if (!Eliminated[j])
                {
                    t = E->Score[x + j] - 0.5;
                    if (t < 0.0)
                        t = -t;
                    s += pow(t, IRNRPOWER);
                }
            if (s > 0.0)
            {
                s = pow(s, -1.0 / IRNRPOWER);
                for (j = E->NumCands - 1; j >= 0; j--)
                    if (!Eliminated[j])
                    {
                        SumNormedRating[j] += s * E->Score[x + j];
                    }
            }
        }
        RandomlyPermute(E->NumCands, RandCandPerm);
        loser = -1;
        minc = HUGE;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            r = RandCandPerm[j];
            if (!Eliminated[r])
            {
                if (SumNormedRating[r] < minc)
                {
                    minc = SumNormedRating[r];
                    loser = r;
                }
            }
        }
        assert(loser >= 0);
        Eliminated[loser] = TRUE;
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find random non-eliminated candidate... */
        j = RandCandPerm[i];
        if (!Eliminated[j])
        {
            return j; /*winner*/
        }
    }
    return (-1); /*error*/
}

/* Like IRNR based on squares, but when "renomalizing" it now does a TWO-parameter
 * linear transformation so mean=0 and variance=1. */
EMETH IRNRv(edata *E /*Brian Olson's voting method but with 2-param renorm*/
)
{ /* side effects: Eliminated[], SumNormedRatings[]*/
    int rd, i, j, x, r, loser, ct;
    real s, t, minc, avg;

    FillBoolArray(E->NumCands, Eliminated, FALSE);
    for (rd = E->NumCands; rd > 1; rd--)
    {
        ZeroRealArray(E->NumCands, SumNormedRating);
        for (i = 0; i < (int)E->NumVoters; i++)
        {
            x = i * E->NumCands;
            s = 0.0;
            ct = 0;
            for (j = E->NumCands - 1; j >= 0; j--)
                if (!Eliminated[j])
                {
                    s += E->Score[x + j];
                    ct++;
                }
            assert(ct > 0);
            avg = s / ct; /*mean*/
            s = 0.0;
            for (j = E->NumCands - 1; j >= 0; j--)
                if (!Eliminated[j])
                {
                    t = E->Score[x + j] - avg;
                    s += t * t;
                }
            if (s > 0.0)
            {
                s = 1.0 / sqrt(s);
                for (j = E->NumCands - 1; j >= 0; j--)
                    if (!Eliminated[j])
                    {
                        SumNormedRating[j] += s * (E->Score[x + j] - avg);
                    }
            }
        }
        RandomlyPermute(E->NumCands, RandCandPerm);
        loser = -1;
        minc = HUGE;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            r = RandCandPerm[j];
            if (!Eliminated[r])
            {
                if (SumNormedRating[r] < minc)
                {
                    minc = SumNormedRating[r];
                    loser = r;
                }
            }
        }
        assert(loser >= 0);
        Eliminated[loser] = TRUE;
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find random non-eliminated candidate... */
        j = RandCandPerm[i];
        if (!Eliminated[j])
        {
            return j; /*winner*/
        }
    }
    return (-1); /*error*/
}

/* Like IRNR based on squares, but when "renomalizing" it now does a TWO-parameter
 * linear transformation so max=1 and min=0. */
EMETH IRNRm(edata *E /*Brian Olson's voting method but with 2-param renorm*/
)
{ /* side effects: Eliminated[], SumNormedRatings[]*/
    int rd, i, j, x, r, loser;
    real s, t, minc, mx, mn;

    FillBoolArray(E->NumCands, Eliminated, FALSE);
    for (rd = E->NumCands; rd > 1; rd--)
    {
        ZeroRealArray(E->NumCands, SumNormedRating);
        for (i = 0; i < (int)E->NumVoters; i++)
        {
            x = i * E->NumCands;
            mx = -HUGE;
            mn = HUGE;
            for (j = E->NumCands - 1; j >= 0; j--)
                if (!Eliminated[j])
                {
                    t = E->Score[x + j];
                    if (t < mn)
                        mn = t;
                    if (t > mx)
                        mx = t;
                }
            s = mx - mn;
            if (s > 0.0)
            {
                s = 1.0 / s;
                for (j = E->NumCands - 1; j >= 0; j--)
                    if (!Eliminated[j])
                    {
                        SumNormedRating[j] += s * (E->Score[x + j] - mn);
                    }
            }
        }
        RandomlyPermute(E->NumCands, RandCandPerm);
        loser = -1;
        minc = HUGE;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            r = RandCandPerm[j];
            if (!Eliminated[r])
            {
                if (SumNormedRating[r] < minc)
                {
                    minc = SumNormedRating[r];
                    loser = r;
                }
            }
        }
        assert(loser >= 0);
        Eliminated[loser] = TRUE;
    }
    for (i = E->NumCands - 1; i >= 0; i--)
    { /* find random non-eliminated candidate... */
        j = RandCandPerm[i];
        if (!Eliminated[j])
        {
            return j; /*winner*/
        }
    }
    return (-1); /*error*/
}

EMETH MCA(edata *E /*canddt with most-2approvals wins if gets >50%, else regular approval-winner wins*/
)
{ /* side effects: MCAVoteCount[] */
    int i, j, x, winner;
    if (ApprovalWinner < 0)
        Approval(E);
    ZeroIntArray(E->NumCands, (int *)MCAVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            MCAVoteCount[j] += E->Approve2[x + j];
        }
    }
    winner = ArgMaxUIntArr(E->NumCands, MCAVoteCount, (int *)RandCandPerm);
    if (2 * MCAVoteCount[winner] > E->NumVoters)
        return (winner);
    return (ApprovalWinner);
}

/***Chris Benham:
This is my suggested rule for picking the two second-round qualifiers
(for use in a top2 runoff) using Approval for the first round:
  "The two finalists are the Approval winner A; and of those candidates
X who would have more approval than A if ballots that make no
approval distinction between A and X were altered to exclusively
approve X, the second qualifier is the X who is most approved on
ballots that don't approve A."
[There might not be a "second qualifier"; then we just make the approval winner win.]
   This is the "instant" version of a (delayed) 2-round system I
suggested a while back.
  This is a big improvement over simply promoting the two most approved
candidates because (a)it kills the strategy of richer factions trying
to capture both runoff spots by running a pair of clones, and (b) it
makes the method much less vulnerable to "turkey raising" strategy
(because voters in the first round can't have their votes do both of
promote one of their sincerely approved candidates and also
a "turkey").
***/
EMETH Benham2AppRunoff(edata *E /*description above*/
)
{
    int i, j, y, r, maxc;
    uint jct, awct, offset;
    if (ApprovalWinner < 0)
        Approval(E);
    RandomlyPermute(E->NumCands, RandCandPerm);
    maxc = -BIGINT;
    j = -1;
    for (i = 0; i < (int)E->NumCands; i++)
    {
        r = RandCandPerm[i];
        y = PairApproval[ApprovalWinner * E->NumCands + r];
        if (ApprovalVoteCount[r] + y > ApprovalVoteCount[ApprovalWinner])
        {
            if ((int)ApprovalVoteCount[r] - y > maxc)
            {
                maxc = ApprovalVoteCount[r] - y;
                j = r;
            }
        }
    }
    if (j < 0)
        return (ApprovalWinner);
    /* now for honest runoff between ApprovalWinner and j */
    awct = 0;
    jct = 0;
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        offset = i * E->NumCands;
        if (E->PerceivedUtility[offset + ApprovalWinner] > E->PerceivedUtility[offset + j])
        {
            awct++;
        }
        else if (E->PerceivedUtility[offset + ApprovalWinner] < E->PerceivedUtility[offset + j])
        {
            jct++;
        }
    }
    if (awct > jct || (awct == jct && RandBool()))
        return (ApprovalWinner);
    return (j);
}

EMETH Benham2AppRunB(edata *E /*same as above, except if no 2nd qualifier then just use top 2 approval finishers; always do runoff*/
)
{ /* side effects: PairApproval[], ASecond */
    uint offset, pwct = 0, wct = 0;
    int i, j, y, r, maxc;
    uint jct, awct;
    if (ApprovalWinner < 0)
        Approval(E);
    RandomlyPermute(E->NumCands, RandCandPerm);
    maxc = -BIGINT;
    j = -1;
    for (i = 0; i < (int)E->NumCands; i++)
    {
        r = RandCandPerm[i];
        y = PairApproval[ApprovalWinner * E->NumCands + r];
        if (ApprovalVoteCount[r] + y > ApprovalVoteCount[ApprovalWinner])
        {
            if ((int)ApprovalVoteCount[r] - y > maxc)
            {
                maxc = ApprovalVoteCount[r] - y;
                j = r;
            }
        }
    }
    if (j < 0)
    { /* no 2nd qualifier*/
        ASecond = -1;
        ASecond = Arg2MaxUIntArr(E->NumCands, ApprovalVoteCount, (int *)RandCandPerm, ApprovalWinner);
        assert(ASecond >= 0);
        for (i = 0; i < (int)E->NumVoters; i++)
        {
            offset = i * E->NumCands;
            if (E->PerceivedUtility[offset + ApprovalWinner] > E->PerceivedUtility[offset + ASecond])
            {
                pwct++;
            }
            else if (E->PerceivedUtility[offset + ApprovalWinner] < E->PerceivedUtility[offset + ASecond])
            {
                wct++;
            }
        }
        if (pwct > wct || (pwct == wct && RandBool()))
            return (ApprovalWinner);
        return (ASecond);
    }
    /* now for honest runoff between ApprovalWinner and j */
    awct = 0;
    jct = 0;
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        offset = i * E->NumCands;
        if (E->PerceivedUtility[offset + ApprovalWinner] > E->PerceivedUtility[offset + j])
        {
            awct++;
        }
        else if (E->PerceivedUtility[offset + ApprovalWinner] < E->PerceivedUtility[offset + j])
        {
            jct++;
        }
    }
    if (awct > jct || (awct == jct && RandBool()))
        return (ApprovalWinner);
    return (j);
}

EMETH CondorcetApproval(edata *E /*Condorcet winner if exists, else use Approval*/
)
{
    if (ApprovalWinner < 0)
        Approval(E);
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (CondorcetWinner >= 0)
        return CondorcetWinner;
    return ApprovalWinner;
}

EMETH Range(edata *E /* canddt with highest average Score wins */
)
{ /* side effects:   RangeVoteCount[], RangeWinner  */
    int i, j, x;
    ZeroRealArray(E->NumCands, RangeVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            RangeVoteCount[j] += E->Score[x + j];
        }
    }
    RangeWinner = ArgMaxRealArr(E->NumCands, RangeVoteCount, (int *)RandCandPerm);
    return (RangeWinner);
}

EMETH RangeN(edata *E /*highest average rounded Score [rded to integer in 0..RangeGranul-1] wins*/
)
{ /* side effects:   RangeNVoteCount[], uses global integer RangeGranul  */
    int i, j, x, winner;
    assert(RangeGranul >= 2);
    assert(RangeGranul <= 10000000);
    ZeroIntArray(E->NumCands, (int *)RangeNVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            RangeNVoteCount[j] += (uint)((E->Score[x + j]) * (RangeGranul - 0.0000000001));
        }
    }
    winner = ArgMaxUIntArr(E->NumCands, RangeNVoteCount, (int *)RandCandPerm);
    return (winner);
}

EMETH Range2Runoff(edata *E /*top-2-runoff, 1stRd=range, 2nd round has fully-honest voting*/
)
{ /* side effects: RSecond */
    int i;
    uint offset, pwct = 0, wct = 0;
    RSecond = -1;
    if (RangeWinner < 0)
        Range(E);
    RandomlyPermute(E->NumCands, RandCandPerm);
    RSecond = Arg2MaxRealArr(E->NumCands, RangeVoteCount, (int *)RandCandPerm, RangeWinner);
    assert(RSecond >= 0);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        offset = i * E->NumCands;
        if (E->PerceivedUtility[offset + RangeWinner] > E->PerceivedUtility[offset + RSecond])
        {
            pwct++;
        }
        else if (E->PerceivedUtility[offset + RangeWinner] < E->PerceivedUtility[offset + RSecond])
        {
            wct++;
        }
    }
    if (pwct > wct || (pwct == wct && RandBool()))
        return (RangeWinner);
    return (RSecond);
}

EMETH ContinCumul(edata *E /* Renormalize scores so sum(over canddts)=1; then canddt with highest average Score wins */
)
{ /* side effects:   CCumVoteCount[] */
    int i, j, x, winner;
    real sum;
    ZeroRealArray(E->NumCands, CCumVoteCount);
    for (i = 0; i < (int)E->NumVoters; i++)
    {
        x = i * E->NumCands;
        sum = 0.0;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            sum += E->Score[x + j];
        }
        if (sum > 0.0)
            sum = 1.0 / sum;
        else
            sum = 0.0;
        for (j = E->NumCands - 1; j >= 0; j--)
        {
            CCumVoteCount[j] += sum * E->Score[x + j];
        }
    }
    winner = ArgMaxRealArr(E->NumCands, CCumVoteCount, (int *)RandCandPerm);
    return (winner);
}

EMETH TopMedianRating(edata *E /* canddt with highest median Score wins */
)
{ /* side effects:   MedianRating[], CScoreVec[]  */
    int i, j, x, winner;
    for (j = E->NumCands - 1; j >= 0; j--)
    {
        for (i = E->NumVoters - 1; i >= 0; i--)
        {
            x = i * E->NumCands + j;
            CScoreVec[i] = E->Score[x];
        }
        MedianRating[j] = TwiceMedianReal(E->NumVoters, CScoreVec);
    }
    winner = ArgMaxRealArr(E->NumCands, MedianRating, (int *)RandCandPerm);
    return (winner);
}

EMETH LoMedianRank(edata *E /* canddt with best median ranking wins */
)
{ /* side effects:   MedianRank[], CRankVec[]  */
    int i, j, x, winner;
    for (j = E->NumCands - 1; j >= 0; j--)
    {
        for (i = E->NumVoters - 1; i >= 0; i--)
        {
            x = i * E->NumCands + j;
            CRankVec[i] = E->CandRankings[x];
        }
        MedianRank[j] = TwiceMedianInt(E->NumVoters, CRankVec);
        assert(MedianRank[j] >= 0);
        assert(MedianRank[j] <= 2 * ((int)E->NumCands - 1));
    }
    winner = ArgMinIntArr(E->NumCands, MedianRank, (int *)RandCandPerm);
    return (winner);
}

/* Tideman ranked pairs with Honest voters.
 * Tideman = Condorcet variant in which you
 * pick the A>B comparison with the largest margin and "lock it in".
 * Then you  pick the next largest one available
 * ("available" means: not already used and not creating a cycle),
 * and continue on.  This creates an ordering of the candidates. The
 * topmost in the ordering wins.  It is a bit tricky to spot
 * the cycles as we go...
 * (This code based on email from Blake Cretney bcretney@postmark.net
 * and runs in order N^4 time with N candidates, i.e slow.  It is possible to speed it
 * up to Otilde(N^2) steps with the use of fast data structures...):
 ******************************************/
EMETH TidemanRankedPairs(edata *E /*lock in comparisons with largest margins not yielding cycle*/
)
{ /*side effects: Tpath[] is used as a changeable copy of MarginsMatrix.*/
    int i, r, j, winner;
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return CondorcetWinner;
    CopyIntArray(SquareInt(E->NumCands), E->MarginsMatrix, Tpath);
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = 0; i < (int)E->NumCands; i++)
    {
        Tpath[i * E->NumCands + i] = BIGINT;
    }
    /* Whenever a victory
   * is locked in, the appropriate Tpath[] cell is set to BIGINT.
   * pi,pj are used with randperm to give precedence to victories higher
   * in the random permutation (when tie-breaking).
   * This loop finds the next pair (i,j) to lock in: ********/
    for (;;)
    {
        int maxp, oi, oj, pi, pj;
        maxp = -BIGINT;
        j = -1;
        for (pi = 0; pi < (int)E->NumCands; pi++)
        {
            oi = RandCandPerm[pi];
            for (pj = pi + 1; pj < (int)E->NumCands; pj++)
            {
                oj = RandCandPerm[pj];
                if (Tpath[oi * E->NumCands + oj] != BIGINT && Tpath[oj * E->NumCands + oi] != BIGINT)
                { /*not locked-out*/
                    if (Tpath[oi * E->NumCands + oj] > maxp)
                    {
                        maxp = Tpath[oi * E->NumCands + oj];
                        i = oi;
                        j = oj;
                    }
                    if (Tpath[oj * E->NumCands + oi] > maxp)
                    {
                        maxp = Tpath[oj * E->NumCands + oi];
                        i = oj;
                        j = oi;
                    }
                }
            }
        }
        if (maxp == -BIGINT)
            break;
        assert(j >= 0);
        /********* print the pair and its margin:
	       printf("(%d %d) %d\n",i,j,maxp);
    ***********************/
        /*lock in the pair and clobber future no-good pairs:*/
        for (oi = 0; oi < (int)E->NumCands; oi++)
            for (oj = 0; oj < (int)E->NumCands; oj++)
            {
                if (Tpath[oi * E->NumCands + i] == BIGINT && Tpath[j * E->NumCands + oj] == BIGINT)
                {
                    Tpath[oi * E->NumCands + oj] = BIGINT;
                }
            }
    }
    /* The above code assumes that pairels has been set properly.
   * Tpath[*E->NumCands +] ends up with the winning row having all cells
   * set to BIGINT.  In fact, a complete ranking is given,
   * where Tpath[i*E->NumCands +j]==BIGINT means that i is
   * ranked over j (where i!=j).   So to find the winner: ****/
    winner = -99;
    for (i = 0; i < (int)E->NumCands; i++)
    {
        r = RandCandPerm[i];
        for (j = 0; j < (int)E->NumCands; j++)
        {
            if (Tpath[r * E->NumCands + j] != BIGINT)
                break;
        }
        if (j >= (int)E->NumCands)
        {
            winner = r;
            break;
        }
    }
    assert(winner >= 0);
    return (winner);
}

EMETH HeitzigRiver(edata *E /*http://lists.electorama.com/pipermail/election-methods-electorama.com/2004-October/013971.html*/
)
{ /* side effects: Hpotpar[], Hpar[], Hrot[] */
    int r, z, i, j, k, pp, oldroot, newroot, maxc;
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return CondorcetWinner;
    /* find potential (and actual) parents: */
    for (i = 0; i < (int)E->NumCands; i++)
    {
        maxc = -BIGINT;
        pp = -1;
        RandomlyPermute(E->NumCands, RandCandPerm);
        for (j = 0; j < (int)E->NumCands; j++)
        {
            r = RandCandPerm[j];
            if (r != i && E->MarginsMatrix[r * E->NumCands + i] > maxc)
            {
                maxc = E->MarginsMatrix[r * E->NumCands + i];
                pp = r;
            }
        }
        assert(pp >= 0);
        Hpotpar[i] = pp;
        Hpar[i] = i;
        Hroot[i] = i;
    }
    for (z = E->NumCands;;)
    { /* loop that adds arcs: */
        /* scan tree roots to find best new arc to add: */
        maxc = -BIGINT;
        k = -1;
        RandomlyPermute(E->NumCands, RandCandPerm);
        for (i = 0; i < (int)E->NumCands; i++)
        {
            r = RandCandPerm[i];
            if (Hpar[r] == r)
            { /* tree root */
                pp = Hpotpar[r];
                if (maxc < E->MarginsMatrix[pp * E->NumCands + r])
                {
                    maxc = E->MarginsMatrix[pp * E->NumCands + r];
                    k = r;
                }
            }
        } /*end for(i)*/
        assert(k >= 0);
        /* add it: */
        pp = Hpotpar[k];
        Hpar[k] = pp;
        /* update roots: */
        newroot = Hroot[pp];
        z--;
        if (z <= 1)
            break;
        oldroot = Hroot[k];
        for (i = 0; i < (int)E->NumCands; i++)
        {
            if (Hroot[i] == oldroot)
                Hroot[i] = newroot;
        }
        /* update potential parent of newroot: */
        maxc = -BIGINT;
        k = -1;
        for (j = 0; j < (int)E->NumCands; j++)
        {
            r = RandCandPerm[j];
            if (Hroot[r] != newroot && E->MarginsMatrix[r * E->NumCands + newroot] > maxc)
            {
                maxc = E->MarginsMatrix[r * E->NumCands + newroot];
                k = r;
            }
        }
        assert(k >= 0);
        Hpotpar[newroot] = k;
    }
    return newroot;
}

EMETH DMC(edata *E /* eliminate least-approved candidate until unbeaten winner exists */
)
{ /* side effects: LossCount[] */
    int i, j, t;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (CWSPEEDUP && CondorcetWinner >= 0)
        return (CondorcetWinner);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        t = 0;
        for (j = E->NumCands - 1; j >= 0; j--)
            if (E->MarginsMatrix[j * E->NumCands + i] > 0)
            {
                t++;
            }
        LossCount[i] = t;
    }
    RandomlyPermute(E->NumCands, RandCandPerm);
    IntPermShellSortDown(E->NumCands, (int *)RandCandPerm, (int *)ApprovalVoteCount);
    for (i = E->NumCands - 1; i > 0; i--)
    {
        if (LossCount[i] <= 0)
        {
            return (i); /*winner*/
        }
        for (j = 0; j < i; j++)
        { /* eliminate i and update Losscount[] */
            t = E->MarginsMatrix[i * E->NumCands + j];
            if (t > 0)
            {
                LossCount[j]--;
            }
        }
    }
    return (i);
}

void BSbeatDFS(int x, int diff, bool Set[], bool OK[], int Mat[], int N)
{
    int i;
    for (i = 0; i < N; i++)
        if (OK[i] && i != x)
        {
            if (Mat[i * N + x] >= diff)
            {
                if (!Set[i])
                {
                    Set[i] = TRUE;
                    BSbeatDFS(i, diff, Set, OK, Mat, N);
                }
            }
        }
}

EMETH BramsSanverPrAV(edata *E /*SJ Brams & MR Sanver: Voting Systems That Combine Approval and Preference,2006*/
)
{ /* side effects: MajApproved[] */
    int i, j, winner, ctm, t, maxt, CopeWinr, r;
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (ApprovalWinner < 0)
        Approval(E);
    ctm = 0;
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        if (2 * ApprovalVoteCount[i] > E->NumVoters)
        {
            MajApproved[i] = TRUE;
            ctm++;
        }
        else
        {
            MajApproved[i] = FALSE;
        }
    }
    if (ctm <= 1)
    {
        /*if exactly 0 or 1 canddt majority-approved, ApprovalWinner wins*/
        return ApprovalWinner;
    }
    assert(ctm >= 2); /*Two or more majority-approved:*/
    for (i = E->NumCands - 1; i >= 0; i--)
        if (MajApproved[i])
        {
            for (j = E->NumCands - 1; j >= 0; j--)
                if (j != i && MajApproved[j])
                {
                    if (E->MarginsMatrix[i * E->NumCands + j] <= 0)
                    {
                        goto BADi;
                    }
                }
            winner = i; /*beats-all winner among >=2 majority-approved canddts wins*/
            return winner;
        BADi:;
        }
    /*now Brams&Sanver want the most-approved member of the 
   *top-cycle among the majority-approved canddts, to win: */
    maxt = 0;
    CopeWinr = -1;
    for (i = E->NumCands - 1; i >= 0; i--)
        if (MajApproved[i])
        {
            t = 0;
            for (j = E->NumCands - 1; j >= 0; j--)
                if (j != i && MajApproved[j])
                {
                    if (E->MarginsMatrix[i * E->NumCands + j] > 0)
                    {
                        t++;
                    }
                }
            if (t >= maxt)
            {
                maxt = t;
                CopeWinr = i;
            }
        }
    assert(CopeWinr >= 0);
    assert(MajApproved[CopeWinr]);
    FillBoolArray(E->NumCands, BSSmithMembs, FALSE);
    BSSmithMembs[CopeWinr] = TRUE;
    BSbeatDFS(CopeWinr, 1, BSSmithMembs, MajApproved, E->MarginsMatrix, E->NumCands);
    assert(BSSmithMembs[CopeWinr]);
    RandomlyPermute(E->NumCands, RandCandPerm);
    winner = -1;
    maxt = 0;
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        r = RandCandPerm[i];
        if (BSSmithMembs[r] && ApprovalVoteCount[r] > maxt)
        {
            maxt = ApprovalVoteCount[r];
            winner = r;
        }
    }
    return winner;
}

EMETH MDDA(edata *E /* approval-count winner among canddts not majority-defeated (or all, if all maj-defeated) */
)
{ /* side effects: MDdisquald[] */
    int i, j, r, dqct, thresh, maxc, winner;
    /*if(CWSPEEDUP && CondorcetWinner >=0 ) return(CondorcetWinner); valid??*/
    if (CopeWinOnlyWinner < 0)
        BuildDefeatsMatrix(E);
    if (ApprovalWinner < 0)
        Approval(E);
    dqct = 0;
    thresh = (E->NumVoters) / 2;
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        MDdisquald[i] = FALSE;
        for (j = E->NumCands - 1; j >= 0; j--)
            if (E->DefeatsMatrix[j * E->NumCands + i] > thresh)
            {
                MDdisquald[i] = TRUE;
                dqct++;
                break;
            }
    }
    if (dqct >= (int)E->NumCands)
    { /*If all disqualified, none are. */
        return (ApprovalWinner);
    }
    winner = -1;
    maxc = 0;
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = E->NumCands - 1; i >= 0; i--)
    {
        r = RandCandPerm[i];
        if (!MDdisquald[r] && (int)ApprovalVoteCount[r] >= maxc)
        {
            maxc = ApprovalVoteCount[r];
            winner = r;
        }
    }
    return (winner);
}

/***Forest Simmons 27 Feb 2007:
UncAAO stands for Uncovered, Approval, Approval Opposition.  
Here's how it works:
For each candidate X, if X is uncovered, then let f(X)=X,
else let f(X) be the candidate against which X has the least approval 
opposition, among those candidates that cover X.
["Approval opposition" of X against Y is the number of ballots on which 
X but not Y is approved.]
Start with the approval winner A and apply the function f repeatedly 
until the output equals the input.  This "fixed point" of f is the 
method winner.
  This method requires a tally of both pairwise approval and pairwise 
ordinal information, but both are efficiently summable in N by N 
matrices, where N is the number of candidates.
  This method (UncAAO) is monotone, clone free, and always picks from the 
uncovered set, which is a subset of the Smith set.
  Zero info strategy is sincere.
Even perfect info incentives for burial and betrayal are practically  nil.
   As near as I can tell, the only bad thing about the method is the 
"tyranny of the majority" problem shared by most, if not all, deterministic methods.
*****/
EMETH UncAAO(edata *E)
{
    int i, j, ff, AppOpp, MnAO, r, winner;
    if (ApprovalWinner < 0)
        Approval(E);
    if (RandomUncoveredMemb < 0)
        UncoveredSet(E);
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (i = (int)E->NumCands - 1; i >= 0; i--)
    {
        if (UncoveredSt[i])
        {
            UncAAOF[i] = i;
        }
        else
        {
            MnAO = BIGINT;
            ff = -1;
            for (j = (int)E->NumCands - 1; j >= 0; j--)
            {
                r = RandCandPerm[j];
                if (CoverMatrix[r * E->NumCands + i])
                {
                    AppOpp = ApprovalVoteCount[r] - PairApproval[r * E->NumCands + i];
                    if (AppOpp < MnAO)
                    {
                        MnAO = AppOpp;
                        ff = r;
                    }
                }
            }
            assert(ff >= 0);
            UncAAOF[i] = ff;
        }
    }
    winner = ApprovalWinner;
    do
    {
        winner = UncAAOF[winner];
    }
    until(winner == UncAAOF[winner]);
    return (winner);
}

/*  Woodall-DAC:
1. Rank-order ballots.
2. A voter "acquiesces"' to a set of candidates if he does not rank any
candidate outside the set higher than any inside the set.
(Every voter acquiesces to the full candidate-set.)
3. Sort all possible sets from most acquiescing voters to fewest.
Going down the list, disqualify every candidate not found in each set (i.e. take
set intersections) unless that
would disqualify all remaining candidates (i.e. would result in the empty set).
Continue until only one candidate
is not disqualified; he is the winner.
*****************************************/
EMETH WoodallDAC(edata *E /*Woodall: Monotonocity of single-seat preferential election rules, Discrete Applied Maths 77 (1997) 81-98.*/
)
{ /*side effects: WoodHashCount[],  WoodHashSet[], WoodSetPerm[] */
    /* Hash Tab entries contain counter and set-code which is a single machine word. */
    int v, c, k, r;
    uint offset, s, x, h, numsets;
    if (E->NumCands > 4 * sizeof(uint))
    {
        printf("WoodallDAC: too many candidates %d to use machine words(%d) to represent sets\n",
               E->NumCands,
               (int)(4 * sizeof(uint)));
        printf("You could rewrite the code to use uint64s to try allow up to 64 canddts\n");
        exit(EXIT_FAILURE);
    }
    for (v = ARTINPRIME - 1; v >= 0; v--)
    {
        WoodHashCount[v] = 0;
        WoodHashSet[v] = 0;
    }
    for (v = E->NumVoters - 1; v >= 0; v--)
    {
        s = 0;
        offset = v * E->NumCands;
        for (c = 0; c < (int)E->NumCands; c++)
        {
            s |= (1 << (E->TopDownPrefs[offset + c]));
            h = s % ARTINPRIME;
            assert(!EmptySet(s));
            for (;;)
            {
                x = WoodHashSet[h];
                if (EmptySet(x))
                { /* insert new set s into hash table */
                    WoodHashSet[h] = s;
                    WoodHashCount[h] = 1;
                    break;
                }
                else if (x == s)
                { /* already there so increment count */
                    WoodHashCount[h]++;
                    break;
                }
                h++; /* hash table collision so walk ("linear probing") */
            }
        }
    }
    numsets = 0;
    for (v = ARTINPRIME - 1; v >= 0; v--)
    {
        if (!EmptySet(WoodHashSet[v]))
        {
            WoodSetPerm[numsets] = v;
            numsets++;
        }
    }
    assert(numsets <= E->NumCands * E->NumVoters);
    IntPermShellSortDown(numsets, (int *)WoodSetPerm, (int *)WoodHashCount);
    s = WoodHashSet[WoodSetPerm[0]];
    assert(!EmptySet(s));
    if (SingletonSet(s))
    {
        goto DONE;
    }
    for (k = 1; k < (int)numsets; k++)
    { /*decreasing order!*/
        h = WoodSetPerm[k];
        x = s & WoodHashSet[h];
        if (!EmptySet(x))
        {
            s = x;
            if (SingletonSet(s))
            {
                goto DONE;
            }
        }
    }
DONE:;
    /*printf("C%d/%d\n", CardinalitySet(s), E->NumCands);*/
    /*It is extremely rare that s is not a singleton set.  In fact may never happen.*/
    RandomlyPermute(E->NumCands, RandCandPerm);
    for (k = E->NumCands - 1; k >= 0; k--)
    {
        r = RandCandPerm[k];
        if ((s >> r) & 1)
        {
            return r; /* return random set-element */
        }
    }
    return (-1); /*failure*/
}

/******** uber-routines which package many voting methods into one: **********/

void PrintMethName(int WhichMeth, bool Padding)
{
    switch (WhichMeth)
    {
    case (0):
        printf("SociallyBest");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (1):
        printf("SociallyWorst");
        if (Padding)
            PrintNSpaces(2);
        break;
    case (2):
        printf("RandomWinner");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (3):
        printf("Plurality");
        if (Padding)
            PrintNSpaces(5);
        break;
    case (4):
        printf("Borda");
        if (Padding)
            PrintNSpaces(8);
        break;
    case (5):
        printf("IRV");
        if (Padding)
            PrintNSpaces(10);
        break;
    case (6):
        printf("Approval");
        if (Padding)
            PrintNSpaces(9);
        break;
    case (7):
        printf("Range");
        if (Padding)
            PrintNSpaces(8);
        break;
    case (8):
        printf("SmithSet");
        if (Padding)
            PrintNSpaces(8);
        break;
    case (9):
        printf("SchwartzSet");
        if (Padding)
            PrintNSpaces(6);
        break;
    case (10):
        printf("AntiPlurality");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (11):
        printf("Top2Runoff");
        if (Padding)
            PrintNSpaces(3);
        break;
        /****** above methods were "Core"; below are optional *****/
    case (12):
        printf("CondorcetLR");
        if (Padding)
            PrintNSpaces(7);
        break;
    case (13):
        printf("SimpsonKramer");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (14):
        printf("Bucklin");
        if (Padding)
            PrintNSpaces(7);
        break;
    case (15):
        printf("Copeland");
        if (Padding)
            PrintNSpaces(6);
        break;
    case (16):
        printf("SimmonsCond");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (17):
        printf("SmithIRV");
        if (Padding)
            PrintNSpaces(8);
        break;
    case (18):
        printf("BTRIRV");
        if (Padding)
            PrintNSpaces(7);
        break;
    case (19):
        printf("DMC");
        if (Padding)
            PrintNSpaces(10);
        break;
    case (20):
        printf("Dabagh");
        if (Padding)
            PrintNSpaces(7);
        break;
    case (21):
        printf("VtForAgainst");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (22):
        printf("SchulzeBeatpaths");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (23):
        printf("PlurIR");
        if (Padding)
            PrintNSpaces(9);
        break;
    case (24):
        printf("Black");
        if (Padding)
            PrintNSpaces(8);
        break;
    case (25):
        printf("RandomBallot");
        if (Padding)
            PrintNSpaces(1);
        break;
    case (26):
        printf("RandomPair");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (27):
        printf("NansonBaldwin");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (28):
        printf("Nauru");
        if (Padding)
            PrintNSpaces(8);
        break;
    case (29):
        printf("TopMedianRating");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (30):
        printf("LoMedianRank");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (31):
        printf("RaynaudElim");
        if (Padding)
            PrintNSpaces(4);
        break;
    case (32):
        printf("ArrowRaynaud");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (33):
        printf("Sinkhorn");
        if (Padding)
            PrintNSpaces(7);
        break;
    case (34):
        printf("KeenerEig");
        if (Padding)
            PrintNSpaces(7);
        break;
    case (35):
        printf("MDDA");
        if (Padding)
            PrintNSpaces(12);
        break;
    case (36):
        printf("VenzkeDisqPlur");
        if (Padding)
            PrintNSpaces(2);
        break;
    case (37):
        printf("CondorcetApproval");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (38):
        printf("UncoveredSet");
        if (Padding)
            PrintNSpaces(4);
        break;
    case (39):
        printf("BramsSanverPrAV");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (40):
        printf("Coombs");
        if (Padding)
            PrintNSpaces(12);
        break;
    case (41):
        printf("Top3IRV");
        if (Padding)
            PrintNSpaces(11);
        break;
    case (42):
        printf("ContinCumul");
        if (Padding)
            PrintNSpaces(7);
        break;
    case (43):
        printf("IterCopeland");
        if (Padding)
            PrintNSpaces(6);
        break;
    case (44):
        printf("HeitzigRiver");
        if (Padding)
            PrintNSpaces(6);
        break;
    case (45):
        printf("MCA");
        if (Padding)
            PrintNSpaces(15);
        break;
    case (46):
        printf("Range3");
        if (Padding)
            PrintNSpaces(12);
        break;
    case (47):
        printf("Range10");
        if (Padding)
            PrintNSpaces(11);
        break;
    case (48):
        printf("HeismanTrophy");
        if (Padding)
            PrintNSpaces(2);
        break;
    case (49):
        printf("BaseballMVP");
        if (Padding)
            PrintNSpaces(4);
        break;
    case (50):
        printf("App2Runoff");
        if (Padding)
            PrintNSpaces(5);
        break;
    case (51):
        printf("Range2Runoff");
        if (Padding)
            PrintNSpaces(3);
        break;
    case (52):
        printf("HeitzigDFC");
        if (Padding)
            PrintNSpaces(5);
        break;
    case (53):
        printf("ArmytagePCSchulze");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (54):
        printf("Hay");
        if (Padding)
            PrintNSpaces(12);
        break;
    case (55):
        printf("HeitzigLFC");
        if (Padding)
            PrintNSpaces(5);
        break;
    case (56):
        printf("Benham2AppRunoff");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (57):
        printf("Benham2AppRunB");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (58):
        printf("WoodallDAC");
        if (Padding)
            PrintNSpaces(5);
        break;
    case (59):
        printf("UncAAO");
        if (Padding)
            PrintNSpaces(9);
        break;
        /****** below methods are "Slow": *****/
    case (NumFastMethods + 0):
        printf("TidemanRankedPairs");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (NumFastMethods + 1):
        printf("IRNR2");
        if (Padding)
            PrintNSpaces(10);
        break;
    case (NumFastMethods + 2):
        printf("IRNR1");
        if (Padding)
            PrintNSpaces(10);
        break;
    case (NumFastMethods + 3):
        printf("IRNR3");
        if (Padding)
            PrintNSpaces(10);
        break;
    case (NumFastMethods + 4):
        printf("IRNR9");
        if (Padding)
            PrintNSpaces(10);
        break;
    case (NumFastMethods + 5):
        printf("IRNRv");
        if (Padding)
            PrintNSpaces(10);
        break;
    case (NumFastMethods + 6):
        printf("IRNRm");
        if (Padding)
            PrintNSpaces(10);
        break;
    case (NumFastMethods + 7):
        printf("Rouse");
        if (Padding)
            PrintNSpaces(13);
        break;
    default:
        printf("[Unsupported voting method %d]", WhichMeth);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
}

void PrintAvailableVMethods()
{
    int i;
    printf("\nAvailable Voting methods:\n");
    fflush(stdout);
    for (i = 0; i < NumMethods; i++)
    {
        printf("%d=", i);
        PrintMethName(i, FALSE);
        printf("\n");
    }
    fflush(stdout);
}

int GimmeWinner(edata *E, int WhichMeth)
{
    int w;
    switch (WhichMeth)
    {
    case (0):
        w = SociallyBest(E);
        break;
    case (1):
        w = SociallyWorst(E);
        break;
    case (2):
        w = RandomWinner(E);
        break;
    case (3):
        w = Plurality(E);
        break;
    case (4):
        w = Borda(E);
        break;
    case (5):
        w = IRV(E);
        break;
    case (6):
        w = Approval(E);
        break;
    case (7):
        w = Range(E);
        break;
    case (8):
        w = SmithSet(E);
        break;
    case (9):
        w = SchwartzSet(E);
        break;
    case (10):
        w = AntiPlurality(E);
        break;
    case (11):
        w = Top2Runoff(E);
        break;
        /****** above methods were "Core"; below are optional *****/
    case (12):
        w = CondorcetLR(E);
        break;
    case (13):
        w = SimpsonKramer(E);
        break;
    case (14):
        w = Bucklin(E);
        break;
    case (15):
        w = Copeland(E);
        break;
    case (16):
        w = SimmonsCond(E);
        break;
    case (17):
        w = SmithIRV(E);
        break;
    case (18):
        w = BTRIRV(E);
        break;
    case (19):
        w = DMC(E);
        break;
    case (20):
        w = Dabagh(E);
        break;
    case (21):
        w = VtForAgainst(E);
        break;
    case (22):
        w = SchulzeBeatpaths(E);
        break;
    case (23):
        w = PlurIR(E);
        break;
    case (24):
        w = Black(E);
        break;
    case (25):
        w = RandomBallot(E);
        break;
    case (26):
        w = RandomPair(E);
        break;
    case (27):
        w = NansonBaldwin(E);
        break;
    case (28):
        w = Nauru(E);
        break;
    case (29):
        w = TopMedianRating(E);
        break;
    case (30):
        w = LoMedianRank(E);
        break;
    case (31):
        w = RaynaudElim(E);
        break;
    case (32):
        w = ArrowRaynaud(E);
        break;
    case (33):
        w = Sinkhorn(E);
        break;
    case (34):
        w = KeenerEig(E);
        break;
    case (35):
        w = MDDA(E);
        break;
    case (36):
        w = VenzkeDisqPlur(E);
        break;
    case (37):
        w = CondorcetApproval(E);
        break;
    case (38):
        w = UncoveredSet(E);
        break;
    case (39):
        w = BramsSanverPrAV(E);
        break;
    case (40):
        w = Coombs(E);
        break;
    case (41):
        w = Top3IRV(E);
        break;
    case (42):
        w = ContinCumul(E);
        break;
    case (43):
        w = IterCopeland(E);
        break;
    case (44):
        w = HeitzigRiver(E);
        break;
    case (45):
        w = MCA(E);
        break;
    case (46):
        RangeGranul = 3;
        w = RangeN(E);
        break;
    case (47):
        RangeGranul = 10;
        w = RangeN(E);
        break;
    case (48):
        w = HeismanTrophy(E);
        break;
    case (49):
        w = BaseballMVP(E);
        break;
    case (50):
        w = App2Runoff(E);
        break;
    case (51):
        w = Range2Runoff(E);
        break;
    case (52):
        w = HeitzigDFC(E);
        break;
    case (53):
        w = ArmytagePCSchulze(E);
        break;
    case (54):
        w = Hay(E);
        break;
    case (55):
        w = HeitzigLFC(E);
        break;
    case (56):
        w = Benham2AppRunoff(E);
        break;
    case (57):
        w = Benham2AppRunB(E);
        break;
    case (58):
        w = WoodallDAC(E);
        break;
    case (59):
        w = UncAAO(E);
        break;
        /****** below methods are "Slow": *****/
    case (NumFastMethods + 0):
        w = TidemanRankedPairs(E);
        break;
    case (NumFastMethods + 1):
        IRNRPOWER = 2.0;
        w = IRNR(E);
        break;
    case (NumFastMethods + 2):
        IRNRPOWER = 1.0;
        w = IRNR(E);
        break;
    case (NumFastMethods + 3):
        IRNRPOWER = 3.0;
        w = IRNR(E);
        break;
    case (NumFastMethods + 4):
        IRNRPOWER = 9.0;
        w = IRNR(E);
        break;
    case (NumFastMethods + 5):
        w = IRNRv(E);
        break;
    case (NumFastMethods + 6):
        w = IRNRm(E);
        break;
    case (NumFastMethods + 7):
        w = Rouse(E);
        break;
    default:
        printf("Unsupported voting method %d\n", WhichMeth);
        fflush(stdout);
        exit(EXIT_FAILURE);
    } /*end switch*/
    if (w < 0 || w >= (int)E->NumCands)
    {
        printf("Voting method %d=", WhichMeth);
        PrintMethName(WhichMeth, FALSE);
        printf(" returned erroneous winner %d\n", w);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    return (w);
}

typedef struct dum2
{
    uint NumVoters;
    uint NumCands;
    uint NumElections;
    real IgnoranceAmplitude;
    real Honfrac;
    real Regret[NumMethods];
    uint RegCount[NumMethods];
    real MeanRegret[NumMethods];
    real SRegret[NumMethods];
    bool Agree[NumMethods * NumMethods];
    uint AgreeCount[NumMethods * NumMethods];
    bool CondAgree[NumMethods];
    uint TrueCondAgreeCount[NumMethods];
    bool TrueCondAgree[NumMethods];
    uint CondAgreeCount[NumMethods];
    int Winners[NumMethods];
} brdata;

/* all arrays here have NumMethods entries */
int FindWinnersAndRegrets(edata *E, brdata *B, bool Methods[])
{
    int m, w, j;
    real r;
    BuildDefeatsMatrix(E);
    FillBoolArray(NumMethods * NumMethods, B->Agree, FALSE);
    FillBoolArray(NumMethods, B->CondAgree, FALSE);
    FillBoolArray(NumMethods, B->TrueCondAgree, FALSE);
    InitCoreElState();
    for (m = 0; m < NumMethods; m++)
    {
        if (Methods[m] || m < NumCoreMethods)
        { /* always run Core Methods */
            w = GimmeWinner(E, m);
            B->Winners[m] = w;
            r = UtilitySum[BestWinner] - UtilitySum[w];
            if (r < 0.0 || BestWinner != B->Winners[0])
            {
                printf("FUCK! major failure, r=%g<0 u[best]=%g u[w]=%g u[w[0]]=%g\n", r, UtilitySum[BestWinner], UtilitySum[w], UtilitySum[B->Winners[0]]);
                printf("w=%d m=%d BestWinner=%d E->NumCands=%d B->Winners[0]=%d\n", w, m, BestWinner, E->NumCands, B->Winners[0]);
                fflush(stdout);
            }
            assert(BestWinner == B->Winners[0]);
            assert(r >= 0.0); /*can only fail if somebody overwrites array...*/
            B->Regret[m] = r;
            WelfordUpdateMeanSD(r, (int *)&(B->RegCount[m]), &(B->MeanRegret[m]), &(B->SRegret[m]));
            for (j = 0; j < m; j++)
                if (Methods[j] || j < NumCoreMethods)
                {
                    if (B->Winners[j] == w)
                    {
                        B->Agree[m * NumMethods + j] = TRUE;
                        B->Agree[j * NumMethods + m] = TRUE;
                        B->AgreeCount[m * NumMethods + j]++;
                        B->AgreeCount[j * NumMethods + m]++;
                        assert(B->AgreeCount[j * NumMethods + m] == B->AgreeCount[m * NumMethods + j]);
                    }
                }
        }
    }
    if (CondorcetWinner >= 0)
    {
        for (m = 0; m < NumMethods; m++)
        {
            if (Methods[m] || m < NumCoreMethods)
            {
                if (B->Winners[m] == CondorcetWinner)
                {
                    B->CondAgree[m] = TRUE;
                    B->CondAgreeCount[m]++;
                }
                if (B->Winners[m] == TrueCW)
                {
                    B->TrueCondAgree[m] = TRUE;
                    B->TrueCondAgreeCount[m]++;
                }
            }
        }
    }
    return (CondorcetWinner);
}

/*************************** VOTING STRATEGIES: ****
all are subroutines with a common format (embraced by the edata structure):

input:
uint NumVoters;
uint NumCands;

real PerceivedUtility[NumVoters*NumCands];   
Entry x*NumCands+y says the utility (a floating point real; greater means better candidate)
of the yth candidate (y=0..NumCands-1) according to voter x, x=0..E->NumVoters-1,
and note these are undistorted by strategies but are distorted by ignorance.

output:
same as the input for VOTING METHODS [other components of edata]
Note, all strategies assume (truthfully???) that the pre-election
polls are a statistical dead heat, i.e. all candidates equally likely to win.
WELL NO: BIASED 1,2,3...
That is done because pre-biased elections are exponentially well-predictable and result in
too little interesting data.
***************************/

/* HonestyStrat, if honfrac=1.0, gives 100% honest voters (who approve
candidates above mean utility, and approve2 candiates >= mean utility for "approved" candidates,
rank-order everybody honestly, and linearly transform range scores to make best=1, worst=0,
rest linearly interpolated).
But if honfrac=0.0 it gives 100% strategic voters who assume that the candidates are
pre-ordered in order of decreasing likelihood of winning, and that chances decline very rapidly.
These voters try to maximize their vote's impact on lower-numbered candidates.
If 0<honfrac<1 then it randomly chooses, independently 
for each voter, whether to make her be honest or
strategic (honesty probability is honfrac).  Note, if the luck is right,
this still can yield 100% strategic or 100% honest voters
also - that is unlikely, but could happen. ***/
void HonestyStrat(edata *E, real honfrac)
{
    int lobd, hibd, v, i, nexti, ACT;
    uint offi, offset, rb;
    real MovingAvg, tmp, MaxUtil, MinUtil, SumU, MeanU, Mean2U, ThisU, RecipDiffUtil;
    assert(E->NumVoters <= MaxNumVoters);
    assert(E->NumCands <= MaxNumCands);
    assert(honfrac >= 0.0);
    assert(honfrac <= 1.0);
    for (v = E->NumVoters - 1; v >= 0; v--)
    {
        offset = v * E->NumCands;
        if (Rand01() < honfrac)
        { /*honest voter*/
            MakeIdentityPerm(E->NumCands, E->TopDownPrefs + offset);
            RealPermShellSortDown(E->NumCands, (int *)E->TopDownPrefs + offset, E->PerceivedUtility + offset);
            assert(IsPerm(E->NumCands, E->TopDownPrefs + offset));
            MaxUtil = -HUGE;
            MinUtil = HUGE;
            SumU = 0.0;
            for (i = E->NumCands - 1; i >= 0; i--)
            {
                offi = offset + i;
                E->CandRankings[offset + E->TopDownPrefs[offi]] = i;
                ThisU = E->PerceivedUtility[offi];
                if (MaxUtil < ThisU)
                {
                    MaxUtil = ThisU;
                }
                if (MinUtil > ThisU)
                {
                    MinUtil = ThisU;
                }
                SumU += ThisU;
            }
            assert(IsPerm(E->NumCands, E->CandRankings + offset));
            RecipDiffUtil = MaxUtil - MinUtil;
            if (RecipDiffUtil != 0.0)
                RecipDiffUtil = 1.0 / RecipDiffUtil;
            MeanU = SumU / E->NumCands;
            Mean2U = 0.0;
            ACT = 0;
            for (i = E->NumCands - 1; i >= 0; i--)
            {
                offi = offset + i;
                ThisU = E->PerceivedUtility[offi];
                E->Score[offi] = (ThisU - MinUtil) * RecipDiffUtil;
                /* mean-based threshold (with coin toss if exactly at thresh) for approvals */
                if (ThisU > MeanU)
                {
                    E->Approve[offi] = TRUE;
                    Mean2U += ThisU;
                    ACT++;
                }
                else if (ThisU < MeanU)
                    E->Approve[offi] = FALSE;
                else
                    E->Approve[offi] = RandBool();
            }
            Mean2U /= ACT;
            for (i = E->NumCands - 1; i >= 0; i--)
            {
                offi = offset + i;
                ThisU = E->PerceivedUtility[offi];
                if (ThisU >= Mean2U)
                    E->Approve2[offi] = TRUE;
                else
                    E->Approve2[offi] = FALSE;
            }
        }
        else
        { /*strategic voter*/
            ACT = 0;
            Mean2U = 0.0;
            MovingAvg = 0.0;
            nexti = -1;
            hibd = E->NumCands - 1;
            lobd = 0;
            for (i = 0; i < (int)E->NumCands; i++)
            {
                offi = offset + i;
                ThisU = E->PerceivedUtility[offi];
                if (i > nexti)
                {
                    nexti++;
                    assert(nexti >= 0);
                    for (; nexti < (int)E->NumCands; nexti++)
                    {
                        tmp = E->PerceivedUtility[offset + nexti];
                        MovingAvg += (tmp - MovingAvg) / (nexti + 1.0);
                        if (fabs(tmp - MovingAvg) > 0.000000000001)
                            break;
                    }
                }
                assert(lobd >= 0);
                assert(hibd >= 0);
                assert(lobd < (int)E->NumCands);
                assert(hibd < (int)E->NumCands);
                if (ThisU > MovingAvg)
                {
                    E->Approve[offi] = TRUE;
                    E->Score[offi] = 1.0;
                    Mean2U += ThisU;
                    ACT++;
                    E->CandRankings[offi] = lobd;
                    lobd++;
                }
                else if (ThisU < MovingAvg)
                {
                    E->Approve[offi] = FALSE;
                    E->Score[offi] = 0.0;
                    E->CandRankings[offi] = hibd;
                    hibd--;
                }
                else
                {
                    rb = RandBool();
                    E->Approve[offi] = rb;
                    E->Score[offi] = 0.5;
                    if (rb)
                    {
                        E->CandRankings[offi] = lobd;
                        lobd++;
                    }
                    else
                    {
                        E->CandRankings[offi] = hibd;
                        hibd--;
                    }
                }
            }
            Mean2U /= ACT;
            for (i = E->NumCands - 1; i >= 0; i--)
            {
                offi = offset + i;
                ThisU = E->PerceivedUtility[offi];
                if (ThisU >= Mean2U)
                {
                    E->Approve2[offi] = TRUE;
                }
                else
                {
                    E->Approve2[offi] = FALSE;
                }
                assert(E->CandRankings[offi] >= 0);
                assert(E->CandRankings[offi] < E->NumCands);
                E->TopDownPrefs[offset + E->CandRankings[offi]] = i;
            }
        } /*end if(...honfrac) else clause*/
    }     /*end for(v)*/
}

/*************************** VOTER IGNORANCE: ***********
input:
uint NumVoters;
uint NumCands;
real IgnoranceAmplitude;

real Utility[NumVoters*NumCands];   
Entry x*NumCands+y says the utility (a floating point real; greater means better candidate)
of the yth candidate (y=0..NumCands-1) according to voter x, x=0..NumVoters-1.

output:
real PerceivedUtility[NumVoters*NumCands];   
Same thing but adjusted by adding voter ignorance

(everything is embraced inside the edata structure)
***************************/

void AddIgnorance(edata *E, real IgnoranceAmplitude)
{
    int i;
    if (IgnoranceAmplitude < 0.0)
    {
        /* negative is flag to cause VARIABLE levels of ignorance depending on voter (stratified) */
        for (i = E->NumVoters * E->NumCands - 1; i >= 0; i--)
        {
            E->PerceivedUtility[i] = ((IgnoranceAmplitude * (2 * i + 1)) / E->NumCands) * RandNormal() + E->Utility[i];
        }
    }
    else
    {
        /* positive is flag to cause CONSTANT level of ignorance across all voters */
        for (i = E->NumVoters * E->NumCands - 1; i >= 0; i--)
        {
            E->PerceivedUtility[i] = IgnoranceAmplitude * RandNormal() + E->Utility[i];
        }
    }
    /* Both positive & negative modes have the same mean ignorance amplitude */
}

/*************************** UTILITY GENERATORS: ***********
input:
uint NumVoters;
uint NumCands;

output:
real Utility[NumVoters*NumCands];   
Entry x*NumCands+y says the utility (a floating point real; greater means better candidate)
of the yth candidate (y=0..NumCands-1) according to voter x, x=0..NumVoters-1.
***************************/

real VoterLocation[MaxNumVoters * MaxNumIssues];
real CandLocation[MaxNumCands * MaxNumIssues];

#define UTGEN void /*allows fgrep UTGEN IEVS.c to find out what utility-generators now available*/

void GenNormalLocations(/*input:*/ uint NumVoters, uint NumCands, uint Issues,
                        /*output:*/ real VoterLocation[], real CandLocation[])
{
    GenRandNormalArr(NumVoters * Issues, VoterLocation);
    GenRandNormalArr(NumCands * Issues, CandLocation);
}

void GenWackyLocations(/*input:*/ uint NumVoters, uint NumCands, uint Issues,
                       /*output:*/ real VoterLocation[], real CandLocation[])
{
    GenRandWackyArr(NumVoters * Issues, VoterLocation);
    GenRandNormalArr(NumCands * Issues, CandLocation);
}

UTGEN GenNormalUtils(edata *E)
{ /* simplest possible utility generator: random normal numbers: */
    GenRandNormalArr(E->NumVoters * E->NumCands, E->Utility);
}

/* if Issues<0 then it uses wacky skew voter distribution instead of normal. Uses Lp distance: */
UTGEN GenIssueDistanceUtils(edata *E, int Issues, real Lp)
{ /* utility = distance-based formula in Issue-space */
    uint offset, off2, y, x;
    real KK;
    if (Issues < 0)
    {
        Issues = -Issues;
        GenWackyLocations(E->NumVoters, E->NumCands, Issues, VoterLocation, CandLocation);
    }
    else
    {
        GenNormalLocations(E->NumVoters, E->NumCands, Issues, VoterLocation, CandLocation);
    }
    KK = 0.6 * Issues;
    for (x = 0; x < E->NumVoters; x++)
    {
        offset = x * E->NumCands;
        off2 = x * Issues;
        for (y = 0; y < E->NumCands; y++)
        {
            E->Utility[offset + y] = 1.0 / sqrt(KK +
                                                LpDistanceSquared(Issues, VoterLocation + off2, CandLocation + y * Issues, Lp));
        }
    }
}

UTGEN GenIssueDotprodUtils(edata *E, uint Issues)
{ /* utility = canddt*voter vector dot-product in Issue-space */
    uint offset, off2, y, x;
    real s;
    GenNormalLocations(E->NumVoters, E->NumCands, Issues, VoterLocation, CandLocation);
    assert(Issues > 0);
    s = 1.0 / sqrt((real)Issues);
    for (x = 0; x < E->NumVoters; x++)
    {
        offset = x * E->NumCands;
        off2 = x * Issues;
        for (y = 0; y < E->NumCands; y++)
        {
            E->Utility[offset + y] = s * DotProd(Issues, VoterLocation + off2, CandLocation + y * Issues);
        }
    }
}

#define NumHilFiles 87
#define NumDebFiles 6
#define MaxNumRanks 339999

char *electionHILnames[NumHilFiles] = {
    "A1.HIL", "A2.HIL", "A3.HIL", "A4.HIL", "A5.HIL", "A6.HIL", "A7.HIL", "A8.HIL", "A9.HIL",
    "A10.HIL", "A11.HIL", "A12.HIL", "A13.HIL", "A14.HIL", "A15.HIL",
    "A16.HIL", "A17.HIL", "A18.HIL", "A19.HIL",
    "A20.HIL", "A21.HIL", "A22.HIL", "A23.HIL", "A24.HIL", "A25.HIL",
    "A26.HIL", "A27.HIL", "A28.HIL", "A29.HIL",
    "A30.HIL", "A31.HIL", "A32.HIL", "A33.HIL", "A34.HIL", "A35.HIL",
    "A48.HIL", "A49.HIL",
    "A50.HIL", "A51.HIL", "A52.HIL", "A53.HIL", "A54.HIL", "A55.HIL",
    "A56.HIL", "A57.HIL", "A58.HIL", "A59.HIL",
    "A60.HIL", "A61.HIL", "A62.HIL", "A63.HIL", "A64.HIL", "A65.HIL",
    "A66.HIL", "A67.HIL", "A68.HIL", "A69.HIL",
    "A70.HIL", "A71.HIL", "A72.HIL", "A73.HIL", "A74.HIL", "A75.HIL",
    "A76.HIL", "A77.HIL", "A78.HIL", "A79.HIL",
    "A80.HIL", "A81.HIL", "A82.HIL", "A83.HIL", "A84.HIL", "A85.HIL",
    "A86.HIL", "A87.HIL", "A88.HIL", "A89.HIL",
    "A90.HIL", "A91.HIL", "A92.HIL", "A93.HIL", "A94.HIL", "A95.HIL",
    "A96.HIL", "A97.HIL", "A98.HIL", "A99.HIL"};

char *electionDEBnames[NumDebFiles] = {
    "DB2001.DEB", "DB2002.DEB", "DB2003.DEB", "DB2004.DEB", "DB2005.DEB", "DB2006.DEB"};

uint NVotersData[NumHilFiles + NumDebFiles], NCandsData[NumHilFiles + NumDebFiles];
uchar ElData[MaxNumRanks];
int NumElectionsLoaded = 0;

#define TOOMANYELVOTERS 7000
#define VERBOSELOAD FALSE

int LoadEldataFiles()
{
    int c, i, j, v, x, y, elcount, votcount, prefcount, ncands, nvoters, nwinners, itcount;
    FILE *fp;
    elcount = 0;
    votcount = 0;
    prefcount = 0;
    itcount = 0;
    printf("Loading %d HIL-format elections files...\n", NumHilFiles);
    for (i = 0; i < NumHilFiles; i++)
    {
        printf("loading %s itcount=%d\n", electionHILnames[i], itcount);
        fp = fopen(electionHILnames[i], "r");
        if (fp == NULL)
        {
            printf("failure to open file %s for read - terminating\n", electionHILnames[i]);
            printf("Tideman election data files can be got from\n");
            printf("  http://rangevoting.org/TidemanData.html\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "%d %d", &ncands, &nwinners);
        if (ncands < 3 || ncands > MaxNumCands)
        {
            printf("bad #candidates %d in %s - terminating\n", ncands, electionHILnames[i]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        if (nwinners < 1 || nwinners >= ncands)
        {
            printf("bad #winners %d in %s - terminating\n", nwinners, electionHILnames[i]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        v = 0;
        do
        {
            for (j = 0; j < ncands; j++)
            {
                ElData[itcount + j] = ncands - 1;
            }
            for (y = 0;;)
            {
                x = 0;
                do
                {
                    c = getc(fp);
                }
                until(isdigit(c));
                do
                {
                    x = x * 10 + c - '0';
                    c = getc(fp);
                } while (isdigit(c));
                if (VERBOSELOAD)
                    printf("%d ", x);
                if (x == 0)
                {
                    break;
                }
                /*Now do something with x>=1 which is preference y>=0 for voter v>=0*/
                if (x > ncands)
                {
                    printf("bad vote %d in %s - terminating\n", x, electionHILnames[i]);
                    exit(EXIT_FAILURE);
                }
                assert(x > 0);
                ElData[itcount + x - 1] = y;
                prefcount++;
                y++;
            }
            itcount += ncands;
            if (itcount + ncands >= MaxNumRanks)
            {
                printf("ran out of space (MaxNumRanks) for rank %d - terminating\n", itcount);
                exit(EXIT_FAILURE);
            }
            if (VERBOSELOAD)
                printf("[%d]\n", v);
            votcount++;
            v++;
        }
        until(x == 0 && y == 0);
        /*Now do something re the election that just ended with v votes*/
        if (v < 3)
        {
            printf("bad #voters %d in %s - terminating\n", v, electionHILnames[i]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        NVotersData[elcount] = v;
        NCandsData[elcount] = ncands;
        elcount++;
        fclose(fp);
    } /*end for(i)*/
    printf("Loading %d DEB-format elections files...\n", NumDebFiles);
    for (i = 0; i < NumDebFiles; i++)
    {
        printf("loading %s itcount=%d\n", electionDEBnames[i], itcount);
        fp = fopen(electionDEBnames[i], "r");
        if (fp == NULL)
        {
            printf("failure to open file %s for read - terminating\n", electionDEBnames[i]);
            printf("Tideman election data files can be got from\n");
            printf("  http://rangevoting.org/TidemanData.html\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        fscanf(fp, "%d %d", &nvoters, &ncands);
        if (nvoters < 4 || nvoters > TOOMANYELVOTERS)
        {
            printf("bad #voters %d in %s - terminating\n", nvoters, electionDEBnames[i]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        if (ncands < 3 || ncands > MaxNumCands || ncands > 9)
        {
            printf("bad #candidates %d in %s - terminating\n", ncands, electionDEBnames[i]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        for (v = 0; v < nvoters; v++)
        {
            for (j = 0; j < ncands; j++)
            {
                ElData[itcount + j] = ncands - 1;
            }
            for (j = 0; j < ncands; j++)
            {
                do
                {
                    c = getc(fp);
                } while (c == '\n' || c == ' ');
                x = c - '0'; /* watch out for c=='-' */
                /*Now do something with j>=0 which is preference x>0 for voter v>=0*/
                if (VERBOSELOAD)
                    putchar(c);
                if (c != '-')
                    ElData[itcount + j] = x - 1;
                prefcount++;
            }
            itcount += ncands;
            if (itcount + ncands >= MaxNumRanks)
            {
                printf("ran out of space (MaxNumRanks) for rank %d - terminating\n", itcount);
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
            if (VERBOSELOAD)
                printf("[%d]\n", v);
            votcount++;
        }
        /* Now do something re the election that just ended with v votes */
        NVotersData[elcount] = v;
        NCandsData[elcount] = ncands;
        elcount++;
        fclose(fp);
    } /*end for(i)*/
    printf("done loading files; loaded %d elections constituting %d prefs and %d votes and %d ranks in all\n",
           elcount, prefcount, votcount, itcount);
    NumElectionsLoaded = elcount;
    assert(NumElectionsLoaded > 0);
    return (elcount);
}

UTGEN GenRealWorldUtils(edata *E)
{ /** based on Tideman election dataset **/
    uint ff, y, x, V, C, VV;
    static int WhichElection = 0, offset = 0;
    real scalefac;
    if (WhichElection >= NumElectionsLoaded)
    {
        WhichElection = 0;
        offset = 0;
    }
    V = NVotersData[WhichElection];
    C = NCandsData[WhichElection];
    assert(C > 2);
    assert(V > 2);
    E->NumCands = C;
    E->NumVoters = 53; /* always will be 53 voters */
    scalefac = 1.0 / sqrt((real)C);
    for (x = 0; x < E->NumVoters; x++)
    {
        ff = x * E->NumCands;
        VV = RandInt(V); /* choose random voter in the real world election */
        VV *= C;
        for (y = 0; y < E->NumCands; y++)
        {
            E->Utility[ff + y] = (E->NumCands - (real)ElData[offset + VV + y] + RandNormal()) * scalefac;
        }
    }
    offset += NVotersData[WhichElection] * NCandsData[WhichElection];
    WhichElection++;
}

void UtilDispatcher(edata *E, int WhichMeth)
{ /*WhichMeth = -1 ==> real world utils*/
    switch (WhichMeth)
    {
    case (-1):
        GenRealWorldUtils(E);
        break;
    case (0):
        GenNormalUtils(E);
        break;
    case (1):
        GenIssueDotprodUtils(E, 1);
        break;
    case (2):
        GenIssueDotprodUtils(E, 2);
        break;
    case (3):
        GenIssueDotprodUtils(E, 3);
        break;
    case (4):
        GenIssueDotprodUtils(E, 4);
        break;
    case (5):
        GenIssueDotprodUtils(E, 5);
        break;
    case (6):
        GenIssueDistanceUtils(E, 1, 1.0);
        break; /* Now using L1 distance */
    case (7):
        GenIssueDistanceUtils(E, 2, 1.0);
        break;
    case (8):
        GenIssueDistanceUtils(E, 3, 1.0);
        break;
    case (9):
        GenIssueDistanceUtils(E, 4, 1.0);
        break;
    case (10):
        GenIssueDistanceUtils(E, 5, 1.0);
        break;
    case (11):
        GenIssueDistanceUtils(E, -1, 2.0);
        break; /* These L2 distance */
    case (12):
        GenIssueDistanceUtils(E, -2, 2.0);
        break;
    case (13):
        GenIssueDistanceUtils(E, -3, 2.0);
        break;
    case (14):
        GenIssueDistanceUtils(E, -4, 2.0);
        break;
    case (15):
        GenIssueDistanceUtils(E, -5, 2.0);
        break;

    default:
        printf("Unsupported util gen %d\n", WhichMeth);
        fflush(stdout);
        exit(EXIT_FAILURE);
    } /*end switch*/
}

void PrintUtilName(int WhichMeth, bool Padding)
{
    switch (WhichMeth)
    {
    case (-1):
        printf("RealWorld");
        if (Padding)
            PrintNSpaces(9);
        break;
    case (0):
        printf("RandomNormalUtils");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (1):
        printf("IssueDotProd[1]");
        if (Padding)
            PrintNSpaces(2);
        break;
    case (2):
        printf("IssueDotProd[2]");
        if (Padding)
            PrintNSpaces(2);
        break;
    case (3):
        printf("IssueDotProd[3]");
        if (Padding)
            PrintNSpaces(2);
        break;
    case (4):
        printf("IssueDotProd[4]");
        if (Padding)
            PrintNSpaces(2);
        break;
    case (5):
        printf("IssueDotProd[5]");
        if (Padding)
            PrintNSpaces(2);
        break;
    case (6):
        printf("IssueDistance[1]");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (7):
        printf("IssueDistance[2]");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (8):
        printf("IssueDistance[3]");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (9):
        printf("IssueDistance[4]");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (10):
        printf("IssueDistance[5]");
        if (Padding)
            PrintNSpaces(0);
        break;
    case (11):
        printf("IssueDistance[-1]");
        if (Padding)
            PrintNSpaces(1);
        break;
    case (12):
        printf("IssueDistance[-2]");
        if (Padding)
            PrintNSpaces(1);
        break;
    case (13):
        printf("IssueDistance[-3]");
        if (Padding)
            PrintNSpaces(1);
        break;
    case (14):
        printf("IssueDistance[-4]");
        if (Padding)
            PrintNSpaces(1);
        break;
    case (15):
        printf("IssueDistance[-5]");
        if (Padding)
            PrintNSpaces(1);
        break;

    default:
        printf("UnsupportedUtilGen[%d]\n", WhichMeth);
        fflush(stdout);
        exit(EXIT_FAILURE);
    } /*end switch*/
}

/************ useful IO stuff... ***********/
void PrintConsts()
{
    printf("\nConstants:\n");
    printf("sizeof(uint)=%d bytes\t", (int)sizeof(uint));
    printf("sizeof(uint32)=%d\t", (int)sizeof(uint32));
    printf("sizeof(uint64)=%d\t", (int)sizeof(uint64));
    printf("sizeof(real)=%d\n", (int)sizeof(real));
    printf("sizeof(edata)=%d\t", (int)sizeof(edata));
    printf("MaxNumCands=%d\t", MaxNumCands);
    printf("MaxNumVoters=%d\t", MaxNumVoters);
    printf("MaxNumIssues=%d\n", MaxNumIssues);
    printf("NumMethods=%d\t", NumMethods);
    printf("NumCoreMethods=%d\t", NumCoreMethods);
    printf("TRUE=%d\t", (int)TRUE);
    printf("FALSE=%d\n", (int)FALSE);
    ARTINPRIME = FindArtinPrime(MaxNumCands * 3 * MaxNumVoters);
    printf("ArtinPrime=%d\n", ARTINPRIME);

    printf("BROutputMode=%x\n", BROutputMode);

    if (sizeof(uint32) != 4)
    {
        printf("uint32 has wrong size - need to change defn so it is 4 bytes = 32 bits. Terminating\n");
        exit(EXIT_FAILURE);
    }
    if (sizeof(uint64) != 8)
    {
        printf("uint64 has wrong size - need to change defn so it is 8 bytes = 64 bits. Terminating\n");
        exit(EXIT_FAILURE);
    }
    fflush(stdout);
}

/************ Bayesian Regret ***********/
void ComputeBRs(brdata *B, bool VotMethods[], int UtilMeth)
{
    uint elnum;
    edata E;

    ZeroRealArray(NumMethods, B->MeanRegret);
    ZeroRealArray(NumMethods, B->SRegret);
    ZeroIntArray(NumMethods, (int *)B->RegCount);
    ZeroIntArray(NumMethods * NumMethods, (int *)B->AgreeCount);
    ZeroIntArray(NumMethods * NumMethods, (int *)B->AgreeCount);
    ZeroIntArray(NumMethods, (int *)B->CondAgreeCount);
    ZeroIntArray(NumMethods, (int *)B->TrueCondAgreeCount);
    InitCoreElState();
    E.NumVoters = B->NumVoters;
    E.NumCands = B->NumCands;
    if (B->NumElections < 1)
    {
        printf("NumElections=%d<1, error\n", B->NumElections);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    for (elnum = 0; elnum < B->NumElections; elnum++)
    {
        UtilDispatcher(&E, UtilMeth);
        AddIgnorance(&E, B->IgnoranceAmplitude);
        HonestyStrat(&E, B->Honfrac);
        FindWinnersAndRegrets(&E, B, VotMethods);
    }
    B->NumVoters = E.NumVoters;
    B->NumCands = E.NumCands;
    ScaleRealVec(NumMethods, B->SRegret, 1.0 / ((B->NumElections - 1.0) * B->NumElections)); /*StdDev/sqrt(#) = StdErr.*/
}

void TestEDataStructs(brdata *B)
{
    uint elnum;
    edata E;
    E.NumVoters = B->NumVoters;
    E.NumCands = B->NumCands;
    if (B->NumElections < 1)
    {
        printf("NumElections=%d<1, error\n", B->NumElections);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    for (elnum = 0; elnum < B->NumElections; elnum++)
    {
        printf("GenNormalUtils:\n");
        fflush(stdout);
        GenNormalUtils(&E);
        printf("AddIgnorance:\n");
        fflush(stdout);
        AddIgnorance(&E, B->IgnoranceAmplitude);
        printf("HonestyStrat:\n");
        fflush(stdout);
        HonestyStrat(&E, 1.0);
        printf("BuildDefetasMatrix:\n");
        fflush(stdout);
        BuildDefeatsMatrix(&E);
        printf("PrintEdata:\n");
        fflush(stdout);
        PrintEdata(stdout, &E);
    }
    fflush(stdout);
}

/*************************** BMP BITMAP GRAPHICS: ***************************/

/**************
Description of MS-Windows bmp format, especially 16 colors uncompressed mode.   
All multibyte numbers are little-endian, meaning the 
least significant byte (LSB) value is at the lowest address. 

File begins with these bytes forming the BITMAPFILEHEADER:
#bytes
2        "BM" = 19778 decimal = 42 4D hex = 66 dec, 77 dec.  Note 19778 = 77*256 + 66.
4        size of file in bytes.  For 4 bits per pixel and 200x200 will be 20118 decimal.
2        zero
2        zero
4        offset from begin of file to bitmap data start in bytes.
               This is 118 decimal for 4 bits per pixel:
              2+4+2+2+4+4+4+4+2+2+4+4+4+4+4+4+64 = 118 bytes.

Then comes the BITMAPINFOHEADER:
4       40 decimal  (in windows) = 28 hex.
4       200 decimal  width of image in pixels (here for 200x200).  At most 32K.
4       200 decimal  height of image in pixels (here for 200x200)  At most 32K.
 The #BYTEs per image row must be a multiple of 4 or else effectively will be due to 0-padding.
 With 200 pixels and 4 bits per pixel that is 100 bytes which is indeed a multiple of 4.
2       1 decimal    #planes
2       4 decimal    bits per pixel, can be 1, 4, 8, or 24.  (maybe also 16 & 32.)
    With 4, the 16 colors are usually the "windows standard 16" 
    (and 0 is black and 15 is white) but this is not necessary.
4       0     (but can be nonzero to indicate compression of various kinds; 2 is for 
          run-length encoded 4-bit-per-pixel; compression not available in windows before windows 3.)
4       x     x=size of bitmap data in bytes, rounded to a multiple of 4 bytes.  
              For 200x200, 16-color, will be 20000 decimal.  
              But often left 0 which is legit for uncompressed images.
4       0     preferred horizontal resolutn in pixels/meter, but often left 0.
4       0     preferred vertical resolution in pixels/meter, but often left 0.
4       0     #colors actually used. For 4-bit per pixel will be <=16.  Generally 1<<bitsperpixel.
4       0     #important colors.  Make same as #colors used, or less.
The previous 6 fields often are left 0.  All 6 were added in windows 3 and used to be 0 earlier.

Then comes the RGBQUAD:
It for 16-color images will be a 16*4=64 bytes, each quadruple giving a color.
[For 2-color images it is 2 quads = 8 bytes; for 256-colors it is 256 quads = 1024 bytes.]
The quads are bl, gr, rd, 00 each.

Most commonly this table would be filled with the 16 windows standard colors in order:
color#  rrggbb
0 0x000000   black
1 0x800000   dark red
2 0x008000   dark green
3 0x808000   dark yellow
4 0x000080   dark blue
5 0x800080   dark purple
6 0x008080   dark aqua
7 0xC0C0C0     light grey
248 0x808080   darker grey
249 0xFF0000   red
250 0x00FF00   green
251 0xFFFF00   yellow
252 0x0000FF   blue
253 0xFF00FF   magenta
254 0x00FFFF   aqua
255 0xFFFFFF   white

It seems better, however, to use the best packing of 16 balls in a 3-cube
as the colorset.  The FCC packing of 14 balls would be eight (+-1, +-1, +-1)
plus six (0, 0, +-1).  
Th. Gensane: Dense Packings of Equal Spheres in a Cube,
The electronic journal of combinatorics 11,1 (2004) #R33, gives the best known
packings for N balls, N<=32 and the coordinates are at
http://www.randomwalk.de/sequences/a084824.txt .

Then comes BYTE:
this is just the pixels, which for 4-bit-per-pixel will be 2 pixels per byte.
[Apparently the hi byte comes first in the row and rows move from left to right?]
Note each row is padded out if necessary to make it a multiple of 4 bytes long.

If we are using type-1or2 compression, then
each run of N pixels is specified by 
1st byte: N
2nd byte: the color's index. In 4-bit-color mode, contains two indices in the two nybbles;
    the even#d pixels (starting with 0) are using the high-order nybble,
    the odd#d  pixels (starting with 1) are using the low-order nybble.
    That permits dithering.
If N=0 that means an escape whose meaning depends on the 2nd byte: 
00=end of line,
01=end of file.
**************************************/

void OutputLittleEndianUint32(uint x, FILE *F)
{
    putc(x % 256, F);
    x /= 256;
    putc(x % 256, F);
    x /= 256;
    putc(x % 256, F);
    x /= 256;
    putc(x % 256, F);
}

void OutputLittleEndianUint16(uint x, FILE *F)
{
    putc(x % 256, F);
    x /= 256;
    putc(x % 256, F);
}

void OutputBarray(uint imgsize, uchar Barray[], FILE *F)
{
    int i;
    assert(imgsize == 20000);
    for (i = 0; i < 20000; i++)
        putc(Barray[i], F);
}

uint ReadPixel(uint x, uint y, uchar Barray[])
{ /*assumes 200x200, 4bits per pixel*/
    int addr;
    uint q;
    addr = 100 * y + x / 2;
    q = Barray[addr];
    if (!(x % 2))
    {
        q >>= 4;
    }
    return (q & 15);
}

uint OutputCompressedBarray(uint imgsize, uchar Barray[], bool really, FILE *F)
{
    int j, N;
    uint bc, pix, oldpix;
    assert(imgsize <= 20000);
    j = 0;
    N = 1;
    bc = 0;
    oldpix = ((Barray[j / 2] >> (4 * (1 - j % 2))) & 15);
    for (j = 1; j < 40000; j++)
    {
        pix = ((Barray[j / 2] >> (4 * (1 - j % 2))) & 15);
        assert(pix < 16);
        if (pix == oldpix)
        {
            N++;
        }
        else
        {
            assert(N < 256);
            if (really)
                putc(N, F); /*run length*/
            assert(oldpix < 16);
            if (really)
                putc(oldpix | (oldpix << 4), F); /*color in both nybbles*/
            N = 1;
            bc += 2;
        }
        if (j % 200 == 199)
        {
            assert(N < 256);
            if (really)
                putc(N, F); /*run length*/
            assert(pix < 16);
            if (really)
                putc(pix | (pix << 4), F); /*color in both nybbles*/
            N = 1;
            bc += 4;
            if (really)
                putc(0, F);
            if (j >= 39999)
            {
                if (really)
                    putc(1, F); /*end of file*/
            }
            else
            {
                if (really)
                    putc(0, F); /*end of line*/
                j++;
                pix = ((Barray[j / 2] >> (4 * (1 - j % 2))) & 15);
            }
        }
        oldpix = pix;
    } /*end for(j)*/
    return bc;
}

uint OutputBMPHead(uint width, uint height, uint bitsperpixel, uint compression, uchar Barray[], FILE *F)
{
    uint sf, roundedwidth, colors, colortabsize, offset, imgsize, compressedsize;
    assert(bitsperpixel == 1 || bitsperpixel == 4 || bitsperpixel == 8 || bitsperpixel == 24);
    putc(66, F); /*B*/
    putc(77, F); /*M*/
    roundedwidth = (width / 2) + 2 * (width % 2);
    if (roundedwidth % 4 != 0)
    {
        roundedwidth = (roundedwidth / 4 + 1) * 4;
    }
    colors = 1 << bitsperpixel;
    colortabsize = 4 * colors;
    if (colortabsize > 1024)
        colortabsize = 0;
    offset = 54 + colortabsize;
    imgsize = roundedwidth * height;
    if (imgsize == 20000 && compression)
    {
        compressedsize = OutputCompressedBarray(imgsize, Barray, FALSE, F);
        if (compressedsize < imgsize)
        {
            imgsize = compressedsize;
            compression = 2;
        }
        else
        {
            compression = 0;
        }
    }
    sf = imgsize + offset;
    OutputLittleEndianUint32(sf, F);
    putc(0, F);
    putc(0, F);
    putc(0, F);
    putc(0, F);
    OutputLittleEndianUint32(offset, F);
    OutputLittleEndianUint32(40, F);
    OutputLittleEndianUint32(width, F);
    OutputLittleEndianUint32(height, F);
    OutputLittleEndianUint16(1, F); /*1 plane*/
    OutputLittleEndianUint16(bitsperpixel, F);
    OutputLittleEndianUint32(compression, F); /*0=uncompressed*/
    OutputLittleEndianUint32(imgsize, F);
    OutputLittleEndianUint32(0, F);
    OutputLittleEndianUint32(0, F);
    OutputLittleEndianUint32(colors, F);
    OutputLittleEndianUint32(colors, F);
    /*The user will now want to 
   *1. output colortabsize worth of palette data.
   *2. output imgsize bytes worth of bitmap data.
   *3. close the file F.
   *But neither is not done by this routine.*/
    return (imgsize);
}

uchar PaletteColorArray[64];

void BogoPutc(uchar x, FILE *F)
{
    static uint i = 0;
    if (i >= 64)
        i = 0;
    PaletteColorArray[i] = x;
    i++;
    if (F != NULL)
        putc(x, F);
}

void OutputGensane16ColorPaletteABC(FILE *F)
{
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(0, F);

    BogoPutc(226, F);
    BogoPutc(0, F);
    BogoPutc(212, F);
    BogoPutc(0, F);
    BogoPutc(212, F);
    BogoPutc(226, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(212, F);
    BogoPutc(226, F);
    BogoPutc(0, F);

    BogoPutc(106, F);
    BogoPutc(113, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(106, F);
    BogoPutc(113, F);
    BogoPutc(0, F);
    BogoPutc(113, F);
    BogoPutc(0, F);
    BogoPutc(106, F);
    BogoPutc(0, F);

    BogoPutc(110, F);
    BogoPutc(103, F);
    BogoPutc(222, F);
    BogoPutc(0, F);
    BogoPutc(103, F);
    BogoPutc(222, F);
    BogoPutc(110, F);
    BogoPutc(0, F);
    BogoPutc(222, F);
    BogoPutc(110, F);
    BogoPutc(103, F);
    BogoPutc(0, F);

    BogoPutc(255, F);
    BogoPutc(146, F);
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(146, F);
    BogoPutc(255, F);
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(255, F);
    BogoPutc(146, F);
    BogoPutc(0, F);

    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
}

void OutputGensane16ColorPaletteACB(FILE *F)
{
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F); /*primary*/
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(0, F);

    BogoPutc(0, F);
    BogoPutc(226, F);
    BogoPutc(212, F);
    BogoPutc(0, F);
    BogoPutc(212, F);
    BogoPutc(0, F);
    BogoPutc(226, F);
    BogoPutc(0, F);
    BogoPutc(226, F);
    BogoPutc(212, F);
    BogoPutc(0, F);
    BogoPutc(0, F);

    BogoPutc(0, F);
    BogoPutc(113, F);
    BogoPutc(106, F);
    BogoPutc(0, F);
    BogoPutc(113, F);
    BogoPutc(106, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(106, F);
    BogoPutc(0, F);
    BogoPutc(113, F);
    BogoPutc(0, F);

    BogoPutc(110, F);
    BogoPutc(222, F);
    BogoPutc(103, F);
    BogoPutc(0, F);
    BogoPutc(103, F);
    BogoPutc(110, F);
    BogoPutc(222, F);
    BogoPutc(0, F);
    BogoPutc(222, F);
    BogoPutc(103, F);
    BogoPutc(110, F);
    BogoPutc(0, F);

    BogoPutc(255, F);
    BogoPutc(255, F);
    BogoPutc(146, F);
    BogoPutc(0, F);
    BogoPutc(146, F);
    BogoPutc(255, F);
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(146, F);
    BogoPutc(255, F);
    BogoPutc(0, F);

    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F); /*black*/
}

void OutputFCC16ColorPalette(FILE *F)
{
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F); /*red*/
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(0, F); /*blue*/
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(0, F); /*green*/

    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(255, F);
    BogoPutc(0, F); /*bluegreen*/
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(255, F);
    BogoPutc(0, F); /*bluered*/
    BogoPutc(255, F);
    BogoPutc(255, F);
    BogoPutc(0, F);
    BogoPutc(0, F); /*redgreen*/

    BogoPutc(255, F);
    BogoPutc(127, F);
    BogoPutc(127, F);
    BogoPutc(0, F);
    BogoPutc(127, F);
    BogoPutc(255, F);
    BogoPutc(177, F);
    BogoPutc(0, F); /*added red to try to make yellower*/
    BogoPutc(127, F);
    BogoPutc(127, F);
    BogoPutc(255, F);
    BogoPutc(0, F);

    BogoPutc(0, F);
    BogoPutc(127, F);
    BogoPutc(127, F);
    BogoPutc(0, F);
    BogoPutc(127, F);
    BogoPutc(0, F);
    BogoPutc(127, F);
    BogoPutc(0, F);
    BogoPutc(127, F);
    BogoPutc(127, F);
    BogoPutc(0, F);
    BogoPutc(0, F);

    BogoPutc(127, F);
    BogoPutc(127, F);
    BogoPutc(127, F);
    BogoPutc(0, F); /*add on dark grey*/
    BogoPutc(191, F);
    BogoPutc(191, F);
    BogoPutc(191, F);
    BogoPutc(0, F); /*add on light grey*/

    BogoPutc(255, F);
    BogoPutc(255, F);
    BogoPutc(255, F);
    BogoPutc(0, F); /*white*/
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F);
    BogoPutc(0, F); /*black*/
}

void CreatePixel(int x, int y, uint color, uchar Barray[])
{ /*Create 16 color pixel at x,y*/
    uint q, addr;
    if (x >= 200 || y >= 200 || x < 0 || y < 0)
        return;  /*auto-clipping to 200x200 region*/
    color &= 15; /*at most 16 colors, like it or not*/
    addr = 100 * y + x / 2;
    if (x % 2)
    { /* use of !(x%2) would be wrong, experimentally verified. */
        q = Barray[addr];
        q &= 0xF0;
        q |= color;
        Barray[addr] = q;
    }
    else
    {
        q = Barray[addr];
        q &= 0x0F;
        q |= color << 4;
        Barray[addr] = q;
    }
}

void DrawCircle(int x, int y, uint radius, uint BorderColor, uint FillColor, uchar Barray[])
{ /*Bresenham alg.*/
    int xd, yd, i, d, mode;
    for (mode = 0; mode <= 1; mode++)
    {
        xd = 0;
        yd = radius;
        d = 3 - (2 * yd);
        while (yd >= xd)
        {
            if (mode == 0)
            { /*fill:*/
                for (i = x - xd + 1; i < x + xd; i++)
                {
                    CreatePixel(i, y + yd, FillColor, Barray);
                }
                for (i = x - xd + 1; i < x + xd; i++)
                {
                    CreatePixel(i, y - yd, FillColor, Barray);
                }
                for (i = x - yd + 1; i < x + yd; i++)
                {
                    CreatePixel(i, y + xd, FillColor, Barray);
                }
                for (i = x - yd + 1; i < x + yd; i++)
                {
                    CreatePixel(i, y - xd, FillColor, Barray);
                }
            }
            else
            { /*border points in 8 octants:*/
                CreatePixel(x - xd, y + yd, BorderColor, Barray);
                CreatePixel(x + xd, y + yd, BorderColor, Barray);
                CreatePixel(x - xd, y - yd, BorderColor, Barray);
                CreatePixel(x + xd, y - yd, BorderColor, Barray);
                CreatePixel(x - yd, y + xd, BorderColor, Barray);
                CreatePixel(x + yd, y + xd, BorderColor, Barray);
                CreatePixel(x - yd, y - xd, BorderColor, Barray);
                CreatePixel(x + yd, y - xd, BorderColor, Barray);
            }
            /*update coords:*/
            if (d < 0)
            {
                d += 4 * xd + 6;
            }
            else
            {
                d += 10 + 4 * (xd - yd);
                yd--;
            }
            xd++;
        } /*end while*/
    }
}

uint SquareUint(uint x) { return x * x; }

void DrawVoronoi(uint NumSites, int xx[], int yy[], uchar Barray[20000], int LpPow)
{
    int x, y, ds, min, col, i;
    if (NumSites > 16)
        NumSites = 16;
    for (x = 0; x < 200; x++)
    {
        for (y = 0; y < 200; y++)
        {
            min = 9999999;
            col = BIGINT;
            for (i = 0; i < NumSites; i++)
            {
                if (LpPow == 2)
                    ds = SquareReal(x - xx[i]) + SquareReal(y - yy[i]);
                else /*1*/
                    ds = fabs(x - xx[i]) + fabs(y - yy[i]);
                if (ds < min)
                {
                    min = ds;
                    col = i;
                }
            }
            assert(col < NumSites);
            CreatePixel(x, y, col, Barray);
        }
    }
    for (i = 0; i < NumSites; i++)
    {
        DrawCircle(xx[i], yy[i], 2, ((i != 15) ? 15 : 14), i, Barray);
    }
}

void DrawFPvor(uint NumSites, int xx[], int yy[], uchar Barray[20000], int LpPow)
{
    uint x, y, ds, mx, col, i;
    if (NumSites > 16)
        NumSites = 16;
    for (x = 0; x < 200; x++)
    {
        for (y = 0; y < 200; y++)
        {
            mx = 0;
            col = BIGINT;
            for (i = 0; i < NumSites; i++)
            {
                if (LpPow == 2)
                    ds = SquareReal(x - xx[i]) + SquareReal(y - yy[i]);
                else /*1*/
                    ds = fabs(x - xx[i]) + fabs(y - yy[i]);
                if (ds >= mx)
                {
                    mx = ds;
                    col = i;
                }
            }
            assert(col < NumSites);
            CreatePixel(x, y, col, Barray);
        }
    }
    for (i = 0; i < NumSites; i++)
    {
        DrawCircle(xx[i], yy[i], 2, ((i != 15) ? 15 : 14), i, Barray);
    }
}

/***
Do equiangular spacing in [0, 180)  and
on each line place voters exactly centrosymmetically.
Use reals.  RandNormalRadius.
Then add 1 voter at the center.

Autopilot:
increase voters 10% each time until too many voters, or until
some weight exceeds runnerup by factor 5 or more.
CreatePixel for winner with maxweight.
***/
void YeePicture(uint NumSites, int MaxK, int xx[], int yy[], int WhichMeth,
                edata *E, uchar Barray[], uint GaussStdDev, real honfrac, int LpPow)
{
    int x, y, k, v, j, ja, i, m, jo, s, maxw, col, w, pass, x0, x1, y0, y1, PreColor;
    uint p0, p1, p2, p3;
    int weight[16];
    uint RandPerm[16];
    real xt, yt, xto, yto, th, ds, ut, rr;
    assert(honfrac >= 0.0);
    assert(honfrac <= 1.0);
    if (NumSites > 16)
        NumSites = 16;
    E->NumCands = NumSites;
    MakeIdentityPerm(NumSites, RandPerm);
    for (pass = 8; pass >= 1; pass /= 2)
    {
        for (y = 0; y < 200; y += pass)
        {
            printf("[%d]", y);
            if ((pass == 1 && y % 10 == 9) ||
                (pass == 2 && y % 20 == 18) ||
                (pass == 4 && y % 40 == 36) ||
                (pass == 8 && y % 80 == 72))
                printf("\n");
            for (x = 0; x < 200; x += pass)
            {
                PreColor = -1;
                if (pass >= 8 || ((x & pass) || (y & pass)))
                {
                    if (x > 0 && y > 0 && x < 200 - pass && y < 200 - pass)
                    {
                        if (pass <= 4)
                        {
                            /*speedup hack: examine previously-computed 4 neighbors*/
                            if (x & pass)
                            {
                                x0 = x - pass;
                                x1 = x + pass;
                            }
                            else
                            {
                                x0 = x - pass;
                                x1 = x;
                            }
                            if (y & pass)
                            {
                                y0 = y - pass;
                                y1 = y + pass;
                            }
                            else
                            {
                                y0 = y - pass;
                                y1 = y;
                            }
                            p0 = ReadPixel(x0, y0, Barray);
                            p1 = ReadPixel(x0, y1, Barray);
                            p2 = ReadPixel(x1, y0, Barray);
                            p3 = ReadPixel(x1, y1, Barray);
                            if (p0 == p1 && p0 == p2 && p0 == p3)
                            { /*if all agree then use common color*/
                                PreColor = p0;
                            }
                        } /*end if(pass)*/
                    }     /*end if(x>0 &&..)*/
                    ZeroIntArray(NumSites, weight);
                    for (k = 10; k < MaxK; k = (k * 11) / 10)
                    { /*try k-voter election*/
                        v = k + k + 1;
                        E->NumVoters = v;
                        j = 0;
                        ja = 0;
                        while (ja < k)
                        { /* generate voter locations and their candidate-utilities */
                            th = (ja * PI) / k;
                            xt = cos(th);
                            yt = sin(th);
                            rr = RandRadialNormal() * GaussStdDev;
                            xt *= rr;
                            yt *= rr; /*xt, yt are offsets of voters from central pixel*/
                            for (s = -1; s <= 1; s++)
                                if (s != 0 || ja == 0)
                                { /*centro-symmetric: s=sign*/
                                    /*printf("k=%d v=%d j=%d ja=%d s=%d x=%f y=%f\n",k,v,j,ja,s,xt*s,yt*s);*/
                                    xto = xt * s + x;
                                    yto = yt * s + y;
                                    jo = j * NumSites;
                                    for (i = 0; i < (int)NumSites; i++)
                                    { /*go thru canddts generating utils for voter j*/
                                        if (LpPow == 2)
                                            ds = SquareReal(xto - xx[i]) + SquareReal(yto - yy[i]);
                                        else /*1*/
                                            ds = 0.7 * SquareReal(fabs(xto - xx[i]) + fabs(yto - yy[i]));
                                        ut = 1.0 / sqrt(12000.0 + ds);
                                        m = i + jo;
                                        E->Utility[m] = ut;
                                        E->PerceivedUtility[m] = ut;
                                    } /*end for(i)*/
                                    j++;
                                } /*end for(s)*/
                            ja++;
                        } /*end while(ja)*/
                        InitCoreElState();
                        HonestyStrat(E, honfrac);
                        BuildDefeatsMatrix(E);
                        SmithIRVwinner = 0;
                        w = GimmeWinner(E, WhichMeth);
                        assert(w >= 0);
                        weight[w] += v;
                        maxw = -BIGINT;
                        for (i = 0; i < (int)NumSites; i++)
                            if (i != w)
                            {
                                if (maxw < weight[i])
                                {
                                    maxw = weight[i];
                                }
                            }
                        if ((PreColor == w && k == 10) ||        /*early break; precolor agree with first election pass*/
                            (weight[w] >= 5 * maxw && k > 16) || /*early break - good confidence*/
                            weight[w] - maxw > MaxK * log(((real)MaxK) / k) * 10.1
                            /*early break - futility*/)
                        {
                            break;
                        }
                    } /*end for(k)*/
                    col = ArgMaxUIntArr(NumSites, (uint *)weight, (int *)RandPerm);
                    CreatePixel(x, y, col, Barray);
                } /*end if(pass)*/
            }     /*end for(x)*/
        }         /*end for(y)*/
    }             /*end for(pass)*/
    for (i = 0; i < (int)NumSites; i++)
    {
        DrawCircle(xx[i], yy[i], 2, ((i != 15) ? 15 : 14), i, Barray);
    }
}

void MakeYeePict(char filename[], int xx[], int yy[], int NumSites, int WhichMeth,
                 uint TopYeeVoters, uint GaussStdDev, real honfrac, int LpPow)
{
    FILE *F;
    uchar Barray[20000];
    edata E;
    int i;
    uint imgsize;
#if MSWINDOWS
    F = fopen(filename, "wb");
#else
    F = fopen(filename, "w");
#endif
    if (F == NULL)
    {
        printf("failed to open %s\n", filename);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    if (WhichMeth == 0 && LpPow == 2)
        DrawVoronoi(NumSites, (int *)xx, (int *)yy, Barray, LpPow);
    else if (WhichMeth == 1 && LpPow == 2)
        DrawFPvor(NumSites, (int *)xx, (int *)yy, Barray, LpPow);
    else
        YeePicture(NumSites, (TopYeeVoters - 1) / 2, xx, yy, WhichMeth, &E, Barray,
                   GaussStdDev, honfrac, LpPow);
    imgsize = OutputBMPHead(200, 200, 4, TRUE, Barray, F);
    OutputFCC16ColorPalette(F);
    if (imgsize >= 20000)
        OutputBarray(imgsize, Barray, F);
    else
        imgsize = OutputCompressedBarray(imgsize, Barray, TRUE, F);
    printf("%s: %d bytes\n", filename, imgsize);
    fclose(F);
    printf("coordinates:\n");
    for (i = 0; i < NumSites; i++)
    {
        printf("(%d,%d)", xx[i], yy[i]);
        if (i < NumSites - 1)
            printf(", ");
        if (i % 8 == 7)
            printf("\n");
    }
    printf("\n");
}

real ColorContrastScore(uint NumSites, int xx[], int yy[])
{
    int i, j;
    real s;
    assert(NumSites <= 16);
    s = 0;
    for (i = 0; i < (int)NumSites; i++)
    {
        for (j = NumSites - 1; j > i; j--)
        {
            s += (AbsInt(PaletteColorArray[i * 4] - PaletteColorArray[j * 4]) +
                  AbsInt(PaletteColorArray[i * 4 + 1] - PaletteColorArray[j * 4 + 1]) +
                  AbsInt(PaletteColorArray[i * 4 + 2] - PaletteColorArray[j * 4 + 2])) /
                 (AbsInt(xx[i] - xx[j]) + AbsInt(yy[j] - yy[j]) + 3.0);
        }
    }
    assert(s > 0.0);
    return s;
}

real ReorderForColorContrast(uint NumSites, int xx[], int yy[])
{
    int i, tm;
    real cs2, cscore;
    uint s1, s2;
    OutputFCC16ColorPalette(NULL);
    cscore = ColorContrastScore(NumSites, xx, yy);
    for (i = 0; i < 9000; i++)
    {
        s1 = RandInt(NumSites);
        s2 = RandInt(NumSites);
        if (s1 != s2)
        {
            tm = xx[s1];
            xx[s1] = xx[s2];
            xx[s2] = tm;
            tm = yy[s1];
            yy[s1] = yy[s2];
            yy[s2] = tm;
            cs2 = ColorContrastScore(NumSites, xx, yy);
            if (cs2 < cscore)
            {
                tm = xx[s1];
                xx[s1] = xx[s2];
                xx[s2] = tm;
                tm = yy[s1];
                yy[s1] = yy[s2];
                yy[s2] = tm;
            }
            else
            {
                cscore = cs2;
            }
        }
    }
    return cscore;
}

/******************** BR driver and output: *************/

int honfraclower = 0, honfracupper = 100;
int candnumlower = 2, candnumupper = 7;
int votnumlower = 2, votnumupper = MaxNumVoters;
int numelections2try = 59;
int utilnumlower = 0, utilnumupper = NumUtilGens;
real HonLevels[] = {1.0, 0.5, 0.0, 0.75, 0.25};
real IgnLevels[] = {0.001, 0.01, 0.1, 1.0, -1.0};
real RegretSum[NumMethods];
int CoombCt[NumMethods];
bool CoombElim[NumMethods];
int MethPerm[NumMethods];
int VMPerm[NumMethods];
real RegretData[MaxScenarios * NumMethods];

/*In IEVS 2.59 with NumElections=2999 and MaxNumVoters=3000, 
 *this driver runs for 80-200 hours
 *on a 2003-era computer, producing several 100 Mbytes output.
 *In IEVS 2.59 and above we include a summarizer so that if
 *you ignore the voluminous output, you still get a nice summary of it at 
 *the end:*/
void BRDriver()
{
    real BPStrength[NumMethods * NumMethods];
    bool VotMethods[NumMethods];
    bool CoombElim[NumMethods];
    int i, j, k, r, prind, whichhonlevel, UtilMeth, minc, coombrd, iglevel, TopMeth;
    uint ScenarioCount = 0;
    real scalefac, reb, maxc;
    brdata B;

    for (iglevel = 0; iglevel < 5; iglevel++)
    {
        for (UtilMeth = 0; UtilMeth < NumUtilGens; UtilMeth++)
            if (UtilMeth >= utilnumlower && UtilMeth <= utilnumupper)
            {
                for (whichhonlevel = 0; whichhonlevel < 5; whichhonlevel++)
                {
                    B.Honfrac = HonLevels[whichhonlevel];
                    if (B.Honfrac * 100 < honfracupper + 0.0001 &&
                        B.Honfrac * 100 > honfraclower - 0.0001)
                    {
                        for (prind = 0; Pow2Primes[prind] < MaxNumVoters; prind++)
                            if (Pow2Primes[prind] <= votnumupper && Pow2Primes[prind] >= votnumlower)
                            {
                                B.NumVoters = Pow2Primes[prind];
                                for (B.NumCands = candnumlower; B.NumCands <= candnumupper; B.NumCands++)
                                {
                                    B.NumElections = numelections2try;
                                    /*1299999=good enough to get all BRs accurate to at least 3 significant digits*/
                                    /*2999=good enough for usually 2 sig figs, and is 400X faster*/
                                    B.IgnoranceAmplitude = IgnLevels[iglevel];
                                    FillBoolArray(NumMethods, VotMethods, TRUE); /*might want to only do a subset... ??*/
                                    printf("\n");
                                    fflush(stdout);
                                    printf("(Scenario#%d:", ScenarioCount);
                                    printf(" UtilMeth=");
                                    PrintUtilName(UtilMeth, FALSE);
                                    printf(" Honfrac=%.2f, NumVoters=%d, NumCands=%d, NumElections=%d, IgnoranceAmplitude=%f)\n",
                                           B.Honfrac, B.NumVoters, B.NumCands,
                                           B.NumElections, B.IgnoranceAmplitude);
                                    if (BROutputMode & (ALLMETHS | TOP10METHS))
                                    {
                                        if (BROutputMode & HTMLMODE)
                                            printf("<tr><th>Voting Method</th><th>Regrets</th><th>#Agreements with ");
                                        else
                                            printf("Voting Method & Regrets & #Agreements with ");
                                        if (BROutputMode & VBCONDMODE)
                                            printf("(vote-based) ");
                                        else
                                            printf("(true-utility-based) ");
                                        printf("Condorcet Winner (when CW exists)");
                                        if (BROutputMode & HTMLMODE)
                                            printf("</th></tr>");
                                        printf("\n");
                                    }
                                    fflush(stdout);
                                    ComputeBRs(&B, VotMethods, UtilMeth);
                                    MakeIdentityPerm(NumMethods, (uint *)MethPerm);
                                    RealPermShellSortUp(NumMethods, MethPerm, B.MeanRegret);
                                    TopMeth = 0;
                                    if (BROutputMode & ALLMETHS)
                                        TopMeth = NumMethods;
                                    if (BROutputMode & TOP10METHS)
                                        TopMeth = 10;
                                    for (i = 0; i < TopMeth; i++)
                                    {
                                        r = i;
                                        if (BROutputMode & SORTMODE)
                                            r = MethPerm[i];
                                        if (BROutputMode & HTMLMODE)
                                            printf("<tr><td>");
                                        printf("%d=", r);
                                        PrintMethName(r, TRUE);
                                        if (BROutputMode & HTMLMODE)
                                            printf("</td><td>");
                                        else if (BROutputMode & TEXMODE)
                                            printf(" & ");
                                        if (BROutputMode & NORMALIZEREGRETS)
                                            printf(" \t %8.5g", B.MeanRegret[r] / B.MeanRegret[2]);
                                        else if (BROutputMode & SHENTRUPVSR)
                                            printf(" \t %8.5g", 100.0 * (1.0 - B.MeanRegret[r] / B.MeanRegret[2]));
                                        else
                                            printf(" \t %8.5g", B.MeanRegret[r]);
                                        if (!(BROutputMode & OMITERRORBARS))
                                        {
                                            if (BROutputMode & HTMLMODE)
                                                printf("&plusmn;");
                                            else if (BROutputMode & TEXMODE)
                                                printf("\\pm");
                                            else
                                                printf("+-");
                                            if (BROutputMode & NORMALIZEREGRETS)
                                                reb = sqrt(B.SRegret[r]) / B.MeanRegret[2];
                                            else if (BROutputMode & SHENTRUPVSR)
                                                reb = 100.0 * sqrt(B.SRegret[r]) / B.MeanRegret[2];
                                            else
                                                reb = sqrt(B.SRegret[r]);
                                            printf("%5.2g", reb);
                                        }
                                        if (BROutputMode & HTMLMODE)
                                            printf("</td><td>");
                                        else if (BROutputMode & TEXMODE)
                                            printf(" & ");
                                        if (BROutputMode & VBCONDMODE)
                                            printf(" \t  %7d", B.CondAgreeCount[r]);
                                        else
                                            printf(" \t  %7d", B.TrueCondAgreeCount[r]);
                                        if (BROutputMode & HTMLMODE)
                                            printf("</td></tr>\n");
                                        else if (BROutputMode & TEXMODE)
                                            printf(" \\\\ \n");
                                        else
                                            printf("\n");
                                    } /*end for(i)*/
                                    for (i = 0; i < NumMethods; i++)
                                    { /*accumulate regret data for later summary*/
                                        RegretData[ScenarioCount * NumMethods + i] =
                                            (B.MeanRegret[i] + 0.00000000001 * Rand01()) / (B.MeanRegret[2] + 0.00000000001);
                                    }
                                    ScenarioCount++;
                                    if (ScenarioCount > MaxScenarios)
                                    {
                                        printf("ScenarioCount=%d exceeded upper limit; terminating\n", ScenarioCount);
                                        fflush(stdout);
                                        exit(EXIT_FAILURE);
                                    }
                                    if (BROutputMode & HTMLMODE)
                                        printf("</td></tr>");
                                    else if (BROutputMode & TEXMODE)
                                        printf(" \\\\ ");
                                    printf("\n");
                                    if ((BROutputMode & OMITERRORBARS) && (BROutputMode & (ALLMETHS | TOP10METHS)))
                                    {
                                        if (BROutputMode & NORMALIZEREGRETS)
                                            reb = sqrt(B.SRegret[2]) / B.MeanRegret[2];
                                        else if (BROutputMode & SHENTRUPVSR)
                                            reb = 100.0 * sqrt(B.SRegret[2]) / B.MeanRegret[2];
                                        else
                                            reb = sqrt(B.SRegret[2]);
                                        printf("ErrorBar for RandomWinner's regret=");
                                        if (BROutputMode & HTMLMODE)
                                            printf("&plusmn;");
                                        else if (BROutputMode & TEXMODE)
                                            printf("\\pm");
                                        else
                                            printf("+-");
                                        printf("%5.2g;\n", reb);
                                        printf("This (experimentally always?) upperbounds the error bar for every other regret.\n");
                                    }
                                    fflush(stdout);

                                    if (BROutputMode & DOAGREETABLES)
                                    {
                                        scalefac = 1.0;
                                        if (B.NumElections > 999)
                                        {
                                            scalefac = 999.5 / B.NumElections;
                                            printf("\nScaling AgreeCounts into 0-999.");
                                        }
                                        printf("\nAGREE 0 ");
                                        for (i = 1; i < NumMethods; i++)
                                        {
                                            printf(" %3d ", i);
                                        }
                                        for (i = 0; i < NumMethods; i++)
                                        {
                                            printf("\n%2d ", i);
                                            for (j = 0; j < NumMethods; j++)
                                            {
                                                if (j == i)
                                                    printf("  *  ");
                                                else
                                                    printf(" %4d", (int)(0.4999 + B.AgreeCount[i * NumMethods + j] * scalefac));
                                            }
                                        }
                                        printf("\n");
                                        fflush(stdout);
                                    }
                                } /*end for(prind)*/
                            }
                    } /*end for(whichhonlevel)*/
                }
            }
    }
    printf("==================SUMMARY OF NORMALIZED REGRET DATA FROM %d SCENARIOS=============\n",
           ScenarioCount);
    /* regret-sum, Coombs, and Schulze beatpaths used as summarizers 
   * since are good for honest voters and cloneproof. */
    printf("1. voting methods sorted by sum of (normalized so RandWinner=1) regrets (best first):\n");
    fflush(stdout);
    for (i = 0; i < NumMethods; i++)
    {
        RegretSum[i] = 0.0;
    }
    for (j = 0; j < (int)ScenarioCount; j++)
    {
        r = j * NumMethods;
        for (i = 0; i < NumMethods; i++)
        {
            RegretSum[i] += RegretData[r + i];
        }
    }
    for (i = 0; i < NumMethods; i++)
    {
        VMPerm[i] = i;
        MethPerm[i] = i;
    }
    RealPermShellSortUp(NumMethods, VMPerm, RegretSum);
    for (i = 0; i < NumMethods; i++)
    {
        r = VMPerm[i];
        printf("%d=", r);
        PrintMethName(r, TRUE);
        printf("\t %g\n", RegretSum[r]);
    }

    printf("\n2. in order of elimination by the Coombs method (worst first):\n");
    fflush(stdout);
    for (i = NumMethods - 1; i >= 0; i--)
    {
        CoombElim[i] = FALSE;
    }
    for (coombrd = NumMethods - 2; coombrd >= 0; coombrd--)
    {
        for (i = NumMethods - 1; i >= 0; i--)
        {
            CoombCt[i] = 0;
        }
        for (j = 0; j < (int)ScenarioCount; j++)
        {
            k = -1;
            r = j * NumMethods;
            maxc = -HUGE;
            for (i = 0; i < NumMethods; i++)
                if (!CoombElim[i])
                {
                    if (RegretData[r + i] >= maxc)
                    {
                        maxc = RegretData[r + i];
                        k = i;
                    }
                }
            assert(k >= 0);
            CoombCt[k]++;
        }
        k = -1;
        j = -1;
        for (i = 0; i < NumMethods; i++)
        {
            if (CoombCt[i] > k)
            {
                k = CoombCt[i];
                j = i;
            }
        }
        assert(j >= 0);
        assert(!CoombElim[j]);
        CoombElim[j] = TRUE;
        printf("%d=", j);
        PrintMethName(j, TRUE);
        printf("\n");
        fflush(stdout);
    }
    for (i = 0; i < NumMethods; i++)
        if (!CoombElim[i])
        {
            printf("Coombs Winner: %d=", i);
            PrintMethName(i, TRUE);
            printf("\n");
            fflush(stdout);
            break;
        }

    printf("\n3. voting methods sorted via Schulze beatpaths ordering (best first):\n");
    fflush(stdout);
    for (i = NumMethods - 1; i >= 0; i--)
    {
        for (j = NumMethods - 1; j >= 0; j--)
            BPStrength[i * NumMethods + j] = 0;
    }
    for (r = 0; r < (int)ScenarioCount; r++)
    {
        k = r * NumMethods;
        for (i = NumMethods - 1; i >= 0; i--)
        {
            for (j = NumMethods - 1; j >= 0; j--)
                if (i != j)
                {
                    BPStrength[i * NumMethods + j] += (RegretData[k + i] < RegretData[k + j]) ? 1 : -1;
                }
        }
    }

    for (i = NumMethods - 1; i >= 0; i--)
    {
        for (j = NumMethods - 1; j >= 0; j--)
            if (i != j)
            {
                for (k = 0; k < NumMethods; k++)
                    if (k != j && k != i)
                    {
                        minc = (int)(BPStrength[j * NumMethods + i]);
                        if (BPStrength[i * NumMethods + k] < minc)
                            minc = (int)BPStrength[i * NumMethods + k];
                        if (BPStrength[j * NumMethods + k] < minc)
                            BPStrength[j * NumMethods + k] = minc;
                    }
            }
    }

    for (i = 0; i < NumMethods; i++)
    {
        for (j = i + 1; j < NumMethods; j++)
        {
            if (BPStrength[MethPerm[j] * NumMethods + MethPerm[i]] >
                BPStrength[MethPerm[i] * NumMethods + MethPerm[j]])
            {
                /*i is not as good as j, so swap:*/
                r = MethPerm[i];
                MethPerm[i] = MethPerm[j];
                MethPerm[j] = r;
            }
        }
        printf("%d=", MethPerm[i]);
        PrintMethName(MethPerm[i], TRUE);
        printf("\n");
        fflush(stdout);
    }
    printf("==========end of summary============\n");
    fflush(stdout);
}

/* Like BRDriver only based on the real world election dataset: */
void RWBRDriver()
{
    real BPStrength[NumMethods * NumMethods];
    bool VotMethods[NumMethods];
    bool CoombElim[NumMethods];
    int i, j, k, r, whichhonlevel, minc, coombrd, iglevel, TopMeth;
    uint ScenarioCount = 0;
    real scalefac, reb, maxc;
    brdata B;

    for (iglevel = 0; iglevel < 4; iglevel++)
    {
        for (whichhonlevel = 0; whichhonlevel < 5; whichhonlevel++)
        {
            B.Honfrac = HonLevels[whichhonlevel];
            if (B.Honfrac * 100 < honfracupper + 0.0001 &&
                B.Honfrac * 100 > honfraclower - 0.0001)
            {
                B.NumElections = numelections2try;
                /*1299999=good enough to get all BRs accurate to at least 3 significant digits*/
                /*2999=good enough for usually 2 sig figs, and is 400X faster*/
                B.IgnoranceAmplitude = IgnLevels[iglevel];
                FillBoolArray(NumMethods, VotMethods, TRUE); /*might want to only do a subset... ??*/
                printf("\n");
                fflush(stdout);
                MakeIdentityPerm(NumMethods, (uint *)MethPerm);
                ComputeBRs(&B, VotMethods, -1);
                RealPermShellSortUp(NumMethods, MethPerm, B.MeanRegret);
                printf("(Scenario#%d:", ScenarioCount);
                printf(" Honfrac=%.2f, NumVoters=%d, NumCands=%d, NumElections=%d, IgnoranceAmplitude=%f)\n",
                       B.Honfrac, B.NumVoters, B.NumCands,
                       B.NumElections, B.IgnoranceAmplitude);
                if (BROutputMode & (ALLMETHS | TOP10METHS))
                {
                    if (BROutputMode & HTMLMODE)
                        printf("<tr><th>Voting Method</th><th>Regrets</th><th>#Agreements with ");
                    else
                        printf("Voting Method & Regrets & #Agreements with ");
                    if (BROutputMode & VBCONDMODE)
                        printf("(vote-based) ");
                    else
                        printf("(true-utility-based) ");
                    printf("Condorcet Winner (when CW exists)");
                    if (BROutputMode & HTMLMODE)
                        printf("</th></tr>");
                    printf("\n");
                }
                fflush(stdout);
                TopMeth = 0;
                if (BROutputMode & ALLMETHS)
                    TopMeth = NumMethods;
                if (BROutputMode & TOP10METHS)
                    TopMeth = 10;
                for (i = 0; i < TopMeth; i++)
                {
                    r = i;
                    if (BROutputMode & SORTMODE)
                        r = MethPerm[i];
                    if (BROutputMode & HTMLMODE)
                        printf("<tr><td>");
                    printf("%d=", r);
                    PrintMethName(r, TRUE);
                    if (BROutputMode & HTMLMODE)
                        printf("</td><td>");
                    else if (BROutputMode & TEXMODE)
                        printf(" & ");
                    if (BROutputMode & NORMALIZEREGRETS)
                        printf(" \t %8.5g", B.MeanRegret[r] / B.MeanRegret[2]);
                    else if (BROutputMode & SHENTRUPVSR)
                        printf(" \t %8.5g", 100.0 * (1.0 - B.MeanRegret[r] / B.MeanRegret[2]));
                    else
                        printf(" \t %8.5g", B.MeanRegret[r]);
                    if (!(BROutputMode & OMITERRORBARS))
                    {
                        if (BROutputMode & HTMLMODE)
                            printf("&plusmn;");
                        else if (BROutputMode & TEXMODE)
                            printf("\\pm");
                        else
                            printf("+-");
                        if (BROutputMode & NORMALIZEREGRETS)
                            reb = sqrt(B.SRegret[r]) / B.MeanRegret[2];
                        else if (BROutputMode & SHENTRUPVSR)
                            reb = 100.0 * sqrt(B.SRegret[r]) / B.MeanRegret[2];
                        else
                            reb = sqrt(B.SRegret[r]);
                        printf("%5.2g", reb);
                    }
                    if (BROutputMode & HTMLMODE)
                        printf("</td><td>");
                    else if (BROutputMode & TEXMODE)
                        printf(" & ");
                    if (BROutputMode & VBCONDMODE)
                        printf(" \t  %7d", B.CondAgreeCount[r]);
                    else
                        printf(" \t  %7d", B.TrueCondAgreeCount[r]);
                    if (BROutputMode & HTMLMODE)
                        printf("</td></tr>\n");
                    else if (BROutputMode & TEXMODE)
                        printf(" \\\\ \n");
                    else
                        printf("\n");
                } /*end for(i)*/
                for (i = 0; i < NumMethods; i++)
                { /*accumulate regret data for later summary*/
                    RegretData[ScenarioCount * NumMethods + i] =
                        (B.MeanRegret[i] + 0.00000000001 * Rand01()) / (B.MeanRegret[2] + 0.00000000001);
                }
                ScenarioCount++;
                if (ScenarioCount > MaxScenarios)
                {
                    printf("ScenarioCount=%d exceeded upper limit; terminating\n", ScenarioCount);
                    fflush(stdout);
                    exit(EXIT_FAILURE);
                }
                if (BROutputMode & HTMLMODE)
                    printf("</td></tr>");
                else if (BROutputMode & TEXMODE)
                    printf(" \\\\ ");
                printf("\n");
                if ((BROutputMode & OMITERRORBARS) && (BROutputMode & (ALLMETHS | TOP10METHS)))
                {
                    if (BROutputMode & NORMALIZEREGRETS)
                        reb = sqrt(B.SRegret[2]) / B.MeanRegret[2];
                    else if (BROutputMode & SHENTRUPVSR)
                        reb = 100.0 * sqrt(B.SRegret[2]) / B.MeanRegret[2];
                    else
                        reb = sqrt(B.SRegret[2]);
                    printf("ErrorBar for RandomWinner's regret=");
                    if (BROutputMode & HTMLMODE)
                        printf("&plusmn;");
                    else if (BROutputMode & TEXMODE)
                        printf("\\pm");
                    else
                        printf("+-");
                    printf("%5.2g;\n", reb);
                    printf("This (experimentally always?) upperbounds the error bar for every other regret.\n");
                }
                fflush(stdout);

                if (BROutputMode & DOAGREETABLES)
                {
                    scalefac = 1.0;
                    if (B.NumElections > 999)
                    {
                        scalefac = 999.5 / B.NumElections;
                        printf("\nScaling AgreeCounts into 0-999.");
                    }
                    printf("\nAGREE 0 ");
                    for (i = 1; i < NumMethods; i++)
                    {
                        printf(" %3d ", i);
                    }
                    for (i = 0; i < NumMethods; i++)
                    {
                        printf("\n%2d ", i);
                        for (j = 0; j < NumMethods; j++)
                        {
                            if (j == i)
                                printf("  *  ");
                            else
                                printf(" %4d", (int)(0.4999 + B.AgreeCount[i * NumMethods + j] * scalefac));
                        }
                    }
                    printf("\n");
                    fflush(stdout);
                }
            }
        } /*end for(whichhonlevel)*/
    }     /*end for(ignlevel)*/
    printf("==================SUMMARY OF NORMALIZED REGRET DATA FROM %d SCENARIOS=============\n",
           ScenarioCount);
    /* regret-sum, Coombs, and Schulze beatpaths used as summarizers 
   * since are good for honest voters and cloneproof. */
    printf("1. voting methods sorted by sum of (normalized so RandWinner=1) regrets (best first):\n");
    fflush(stdout);
    for (i = 0; i < NumMethods; i++)
    {
        RegretSum[i] = 0.0;
    }
    for (j = 0; j < (int)ScenarioCount; j++)
    {
        r = j * NumMethods;
        for (i = 0; i < NumMethods; i++)
        {
            RegretSum[i] += RegretData[r + i];
        }
    }
    for (i = 0; i < NumMethods; i++)
    {
        VMPerm[i] = i;
        MethPerm[i] = i;
    }
    RealPermShellSortUp(NumMethods, VMPerm, RegretSum);
    for (i = 0; i < NumMethods; i++)
    {
        r = VMPerm[i];
        printf("%d=", r);
        PrintMethName(r, TRUE);
        printf("\t %g\n", RegretSum[r]);
    }

    printf("\n2. in order of elimination by the Coombs method (worst first):\n");
    fflush(stdout);
    for (i = NumMethods - 1; i >= 0; i--)
    {
        CoombElim[i] = FALSE;
    }
    for (coombrd = NumMethods - 2; coombrd >= 0; coombrd--)
    {
        for (i = NumMethods - 1; i >= 0; i--)
        {
            CoombCt[i] = 0;
        }
        for (j = 0; j < (int)ScenarioCount; j++)
        {
            k = -1;
            r = j * NumMethods;
            maxc = -HUGE;
            for (i = 0; i < NumMethods; i++)
                if (!CoombElim[i])
                {
                    if (RegretData[r + i] >= maxc)
                    {
                        maxc = RegretData[r + i];
                        k = i;
                    }
                }
            assert(k >= 0);
            CoombCt[k]++;
        }
        k = -1;
        j = -1;
        for (i = 0; i < NumMethods; i++)
        {
            if (CoombCt[i] > k)
            {
                k = CoombCt[i];
                j = i;
            }
        }
        assert(j >= 0);
        assert(!CoombElim[j]);
        CoombElim[j] = TRUE;
        printf("%d=", j);
        PrintMethName(j, TRUE);
        printf("\n");
        fflush(stdout);
    }
    for (i = 0; i < NumMethods; i++)
        if (!CoombElim[i])
        {
            printf("Coombs Winner: %d=", i);
            PrintMethName(i, TRUE);
            printf("\n");
            fflush(stdout);
            break;
        }

    printf("\n3. voting methods sorted via Schulze beatpaths ordering (best first):\n");
    fflush(stdout);
    for (i = NumMethods - 1; i >= 0; i--)
    {
        for (j = NumMethods - 1; j >= 0; j--)
            BPStrength[i * NumMethods + j] = 0;
    }
    for (r = 0; r < (int)ScenarioCount; r++)
    {
        k = r * NumMethods;
        for (i = NumMethods - 1; i >= 0; i--)
        {
            for (j = NumMethods - 1; j >= 0; j--)
                if (i != j)
                {
                    BPStrength[i * NumMethods + j] += (RegretData[k + i] < RegretData[k + j]) ? 1 : -1;
                }
        }
    }

    for (i = NumMethods - 1; i >= 0; i--)
    {
        for (j = NumMethods - 1; j >= 0; j--)
            if (i != j)
            {
                for (k = 0; k < NumMethods; k++)
                    if (k != j && k != i)
                    {
                        minc = (int)BPStrength[j * NumMethods + i];
                        if (BPStrength[i * NumMethods + k] < minc)
                            minc = (int)BPStrength[i * NumMethods + k];
                        if (BPStrength[j * NumMethods + k] < minc)
                            BPStrength[j * NumMethods + k] = minc;
                    }
            }
    }

    for (i = 0; i < NumMethods; i++)
    {
        for (j = i + 1; j < NumMethods; j++)
        {
            if (BPStrength[MethPerm[j] * NumMethods + MethPerm[i]] >
                BPStrength[MethPerm[i] * NumMethods + MethPerm[j]])
            {
                /*i is not as good as j, so swap:*/
                r = MethPerm[i];
                MethPerm[i] = MethPerm[j];
                MethPerm[j] = r;
            }
        }
        printf("%d=", MethPerm[i]);
        PrintMethName(MethPerm[i], TRUE);
        printf("\n");
        fflush(stdout);
    }
    printf("==========end of summary============\n");
    fflush(stdout);
}

/*************************** MAIN CODE: ***************************/

void main()
{
    uint seed, choice, ch2, ch3;
    int ihonfrac, TopYeeVoters, GaussStdDev, subsqsideX, subsqsideY, LpPow;
    int WhichMeth, NumSites, i, j;
    int xx[16], yy[16];
    real cscore;
    char fname[100];
    brdata B;

    printf("IEVS (Warren D. Smith's infinitely extendible voting system comparator) at your service!\n");
    printf("Version=%f  Year=%d  Month=%d\n", VERSION, VERSIONYEAR, VERSIONMONTH);
    fflush(stdout);
    PrintConsts();
    printf("\nPlease enter random seed (0 causes machine to auto-generate from TimeOfDay)\n");
    fflush(stdout);
    scanf("%u", &seed);
    InitRand(seed);

    BuildLCMfact();
    assert(SingletonSet(8));
    assert(SingletonSet(256));
    assert(!SingletonSet(256 + 8));
    assert(!SingletonSet(256 + 512));
    assert(!SingletonSet(3));
    assert(!SingletonSet(7));
    assert(!SingletonSet(5));
    assert(!SingletonSet(10));
    assert(!EmptySet(5));
    assert(EmptySet(0));

    printf("What do you want to do?\n1=BayesianRegrets\n2=YeePicture\n");
    printf("3=Test RandGen (and other self-tests)\n");
    printf("4=Tally an election with votes you enter\n");
    do
    {
        fflush(stdout);
        scanf("%u", &choice);
        switch (choice)
        {
        case (1):
            printf("Answer a sequence of questions indicating what output format you want for\n");
            printf("the regret tables:\n");
            printf("I. voting methods (1) sorted-by-regret or (2) by voting-method-number?\n");
            printf("[The latter, while arbitrary, has the advantage of invariance throughout the run.]\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("sorted by regrets.\n");
                    BROutputMode |= SORTMODE;
                    break;
                case (2):
                    printf("sorting by voting method number.\n");
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("II. output (1) plain ASCII (2) TeX table formatting (3) HTML table formatting?\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("plain ASCII.\n");
                    break;
                case (2):
                    printf("TeX.\n");
                    BROutputMode |= TEXMODE;
                    break;
                case (3):
                    printf("HTML.\n");
                    BROutputMode |= HTMLMODE;
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("III. BRs (1) plain (2) normalized so SociallyBest=0, RandomWinner=1\n");
            printf("     (3) normalized so SociallyBest=100, RandomWinner=0, WorseThanRandom<0?\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("plain.\n");
                    break;
                case (2):
                    printf("Best=0, Random=1.\n");
                    BROutputMode |= NORMALIZEREGRETS;
                    break;
                case (3):
                    printf("Best=100, Random=0.\n");
                    BROutputMode |= SHENTRUPVSR;
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("IV. Error bars (1) on every BR value (2) omit & only compute for RandomWinner\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("all error bars.\n");
                    break;
                case (2):
                    printf("omit error bars.\n");
                    BROutputMode |= OMITERRORBARS;
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("V. Print Agreement counts with (1) true-utility(undistorted) Condorcet Winners, (2) vote-based CWs\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("true-utility CWs.\n");
                    break;
                case (2):
                    printf("vote-based CWs.\n");
                    BROutputMode |= VBCONDMODE;
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("VI. Print out intermethod winner-agreement-count tables (1) no, (2) yes\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("NO agree-count tables.\n");
                    break;
                case (2):
                    printf("Yes agree-count tables.\n");
                    BROutputMode |= DOAGREETABLES;
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("VII. Print out regrets for (1) no, (2) only best 10, (3) all methods\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("No regrets printed (minimum verbosity).\n");
                    break;
                case (2):
                    printf("Top10 methods regrets only printed.\n");
                    BROutputMode |= TOP10METHS;
                    break;
                case (3):
                    printf("All regrets printed (maximum verbosity).\n");
                    BROutputMode |= ALLMETHS;
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("VIII. (1) All parameter knob-settings, or (2) restricted ranges?\n");
            honfraclower = 0;
            honfracupper = 100;
            candnumlower = 2;
            candnumupper = 7;
            votnumlower = 2;
            votnumupper = MaxNumVoters;
            numelections2try = 59;
            do
            {
                fflush(stdout);
                scanf("%u", &ch3);
                switch (ch3)
                {
                case (1):
                    printf("All settings.\n");
                    break;
                case (2):
                    printf("Restricted Ranges...\n");
                    printf("Honesty fraction range - default is 0 100:\n");
                    scanf("%d %d", &honfraclower, &honfracupper);
                    if (honfraclower < 0)
                        honfraclower = 0;
                    if (honfracupper > 100)
                        honfracupper = 100;
                    printf("Honesty fraction range [%d, %d] chosen.\n", honfraclower, honfracupper);
                    printf("Candidate Number range - default is 2 7 [but this range ignored if real-world dataset]:\n");
                    scanf("%d %d", &candnumlower, &candnumupper);
                    if (candnumlower < 2)
                        candnumlower = 2;
                    if (candnumupper >= MaxNumCands)
                        candnumupper = MaxNumCands - 1;
                    printf("Candidate number range [%d, %d] chosen.\n", candnumlower, candnumupper);
                    printf("Voter Number range - default is 2 %d [but this range ignored if real-world dataset:\n",
                           votnumupper);
                    scanf("%d %d", &votnumlower, &votnumupper);
                    if (votnumlower < 0)
                        votnumlower = 0;
                    if (votnumupper >= MaxNumVoters)
                        votnumupper = MaxNumVoters;
                    printf("Voter number range [%d, %d] chosen.\n", votnumlower, votnumupper);
                    printf("Number of elections to try per scenario - default is %d\n", numelections2try);
                    scanf("%d", &numelections2try);
                    if (numelections2try < 29)
                        numelections2try = 29;
                    if (numelections2try > 99999999)
                        numelections2try = 99999999;
                    printf("Trying %d elections per scenario.\n", numelections2try);
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("IX. (1) Machine or (2) Real-world-based utilities?\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("Machine.\n");
                    if (ch3 == 2)
                    {
                        printf("Select which utility-generators you want (default 0 thru 15):\n");
                        for (i = 0; i < 16; i++)
                        {
                            printf("%2d: ", i);
                            PrintUtilName(i, TRUE);
                            printf("\n");
                        }
                        scanf("%d %d", &utilnumlower, &utilnumupper);
                        if (utilnumlower < 0)
                            utilnumlower = 0;
                        if (utilnumupper >= 15)
                            utilnumupper = 15;
                        printf("Utility gens  [%d, %d] chosen.\n", utilnumlower, utilnumupper);
                        /**** if ??? 
	    printf("Select LPpow???d):\n");
	    scanf("%d", &LPpow);
	    if(LPpow<1) LPpow=1;
	    if(LPpow>5) LPpow=5;
	    printf("Using L%d distances.\n", LPpow);
	    *****/
                    }
                    BRDriver();
                    break;
                case (2):
                    printf("Real-world-based.\n");
                    LoadEldataFiles();
                    RWBRDriver();
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            break;
        case (2):
            printf("Which voting method? Your choices:");
            PrintAvailableVMethods();
            fflush(stdout);
            scanf("%d", &WhichMeth);
            printf("using %d=", WhichMeth);
            PrintMethName(WhichMeth, FALSE);
            printf(".\nWhat filename [.bmp suffix will be auto-added for you]?\n");
            fflush(stdout);
            scanf("%s", fname);
            for (i = 0; fname[i]; i++)
                ;
            if (i > 30)
            {
                printf("filename too long, moron\n");
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
            fname[i++] = '.';
            fname[i++] = 'b';
            fname[i++] = 'm';
            fname[i++] = 'p';
            fname[i++] = 0;
            printf("how many point-sites do you want [1 to 16]?\n");
            fflush(stdout);
            scanf("%d", &NumSites);
            if (NumSites < 1 || NumSites > 16)
            {
                printf("out of bounds value %d moron, using 16 instead\n", NumSites);
                NumSites = 16;
            }
            printf("Do you want to:\n1. enter the %d coord-pairs yourself;\n", NumSites);
            printf("2. random coordinate auto-generation?\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("Enter coord pairs X Y with space (not comma) between X & Y, newline between pairs\n");
                    printf("(0,0) is in the lower left.  For example an equilateral triangle would be\n");
                    printf("99 197\n186 47\n12 47\nCoords outside of the [[0,199] range are permitted.\nYour coords:\n");
                    fflush(stdout);
                    for (i = 0; i < NumSites; i++)
                    {
                        scanf("%d %d", &(xx[i]), &(yy[i]));
                    }
                    printf("Your coords are:\n");
                    for (i = 0; i < NumSites; i++)
                    {
                        printf("(%d, %d)\n", xx[i], yy[i]);
                    }
                    break;
                case (2):
                    printf("X Y sidelengths of subsquare in which you want the random points (200 for full square):\n");
                    fflush(stdout);
                    scanf("%d %d", &subsqsideX, &subsqsideY);
                    if (subsqsideX <= 0 || subsqsideX >= 200)
                    {
                        subsqsideX = 200;
                    }
                    if (subsqsideY <= 0 || subsqsideY >= 200)
                    {
                        subsqsideY = 200;
                    }
                    printf("using %dx%d centered subrectangle\n", subsqsideX, subsqsideY);
                    for (i = 0; i < NumSites; i++)
                    {
                    REGEN:
                        xx[i] = (int)(100 - subsqsideX * 0.5 + Rand01() * (subsqsideX - 0.001));
                        yy[i] = (int)(100 - subsqsideY * 0.5 + Rand01() * (subsqsideY - 0.001));
                        for (j = 0; j < i; j++)
                        {
                            if (AbsInt(xx[i] - xx[j]) + AbsInt(yy[i] - yy[j]) <= (7 * subsqsideX + 7 * subsqsideY) / 400)
                            {
                                /*too close to some previous point*/
                                goto REGEN;
                            }
                        }
                    }
                    cscore = ReorderForColorContrast(NumSites, xx, yy);
                    printf("Color score %f (big=more constrast); Your coords are:\n", cscore);
                    for (i = 0; i < NumSites; i++)
                    {
                        printf("(%d, %d)\n", xx[i], yy[i]);
                    }
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);
            printf("Do you want IEVS to re-order the points to try for maximum color-contrast? (1) yes (2) no\n");
            do
            {
                fflush(stdout);
                scanf("%u", &ch2);
                switch (ch2)
                {
                case (1):
                    printf("Reordering...\n");
                    cscore = ReorderForColorContrast(NumSites, xx, yy);
                    printf("Color score %f (big=more constrast); Your (reordered) coords are:\n", cscore);
                    for (i = 0; i < NumSites; i++)
                    {
                        printf("(%d, %d)\n", xx[i], yy[i]);
                    }
                    break;
                case (2):
                    printf("OK, leaving points ordered as is.\n");
                    break;
                default:
                    printf("Wrong choice %d, moron - try again\n", ch2);
                    continue;
                }
            } while (FALSE);

            printf("What max election size (#voters) would you like?\n");
            printf("256 recommended as good compromise between speed and randomness.\n");
            printf("You're allowed to go as high as %d (for slowest speed).\n", MaxNumVoters);
            printf("Algorithm keeps redoing elections with 10%% more voters each time until\n");
            printf("either confident know the winner, or reach this voter# bound.\n");
            do
            {
                fflush(stdout);
                scanf("%d", &TopYeeVoters);
                if (TopYeeVoters <= 0 || TopYeeVoters > MaxNumVoters)
                {
                    printf("%d out of range, moron - try again\n", TopYeeVoters);
                    continue;
                }
            } while (FALSE);
            printf("Using TopYeeVoters=%d.\n", TopYeeVoters);
            printf("What standard deviation on the 1D gaussian do you want? (Whole picture width is 200.)\n");
            do
            {
                fflush(stdout);
                scanf("%d", &GaussStdDev);
                if (GaussStdDev <= 0 || GaussStdDev > 999)
                {
                    printf("%d out of range, moron - try again\n", GaussStdDev);
                    continue;
                }
            } while (FALSE);
            printf("Using GaussStdDevX=%d.\n", GaussStdDev);
            printf("What honesty-percentage do you want? (0 to 100.)\n");
            do
            {
                fflush(stdout);
                scanf("%d", &ihonfrac);
                if (ihonfrac <= 0 || ihonfrac > 100)
                {
                    printf("%d out of range, moron - try again\n", ihonfrac);
                    continue;
                }
            } while (FALSE);
            printf("Using honfrac=%d%%.\n", ihonfrac);
            printf("Utilities based on (1) L1 or (2) L2 distance?\n");
            do
            {
                fflush(stdout);
                scanf("%d", &LpPow);
                if (LpPow <= 0 || LpPow > 2)
                {
                    printf("%d out of range, moron - try again\n", LpPow);
                    continue;
                }
            } while (FALSE);
            printf("Using LpPow=%d.\n", LpPow);
            printf("grinding...\n");
            fflush(stdout);
            MakeYeePict(fname, xx, yy, NumSites, WhichMeth, TopYeeVoters, GaussStdDev, ihonfrac * 0.01, LpPow);
            printf("seed=%d\n", seed);
            break;
        case (3):
            printf("Test of randgen & other tests\n");
            TestsOfRand();
            printf("\nTest edata structure:\n");
            fflush(stdout);
            B.NumVoters = 6;
            B.NumCands = 5;
            B.NumElections = 1;
            B.IgnoranceAmplitude = 0.001;
            TestEDataStructs(&B);
            break;
        case (4):
            printf("Tally an election with votes you enter (NOT IMPLEMENTED HERE - try\n");
            printf("http://RangeVoting.org/VoteCalc.html)\n");
            break;
        default:
            printf("Wrong choice %d, moron - try again\n", choice);
            continue;
        }
    } while (FALSE); /* end switch */
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

/****Chris Benham's FBC-satisfying 3-slot method:
* http://lists.electorama.com/pipermail/election-methods-electorama.com/2007-January/019160.html
* 1. Voters give each candidate a top rating , a middle rating or no 
* rating.
*
* 2. Fix the winning threshold T at 50% of the total valid ballots. Give 
* each candidate a score equal to
* the number of ballots on which it is top-rated. If the candidate X 
* with the highest score has a score
* equal or greater than  T, elect  X.
*
* 3. If not, eliminate the (remaining) candidate which is given a top or 
* middle rating on the fewest ballots, and
* on ballots that now top-rate none of the remaining candidates promote 
* all the middle-rated candidates to "top-rated"
* and accordingly amend the scores.
*
* 4. Again, if the now highest scoring candidate X has a score of at 
* least T then elect X. (T does not shrink as ballots 'exhaust').
*
* 5. Repeat steps 3 and 4 until there is a winner. If  no candidate ever 
* reaches a score of T, elect the candidate
* that is top or middle rated on the most ballots (i.e. the Approval winner).
* NOTES:
* we use top-rank as"top", "approval" as middle, and disapproval as unrated.
*
* Note: later Benham abandoned this method and no longer recommends it; it does not 
* actually obey FBC.
*****/
