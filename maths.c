#include "in4073.h"

int iPow(int a, int e) {
    int r = 1;
    if (e == 0) return r;
    while (e != 0) {
        if ((e & 1) == 1) r *= a;
        e >>= 1;
        a *= a;
    }
    return r;
}

int root(int a, int n) {
    int v = 1, bit, tp, t;
    if (n == 0) return 0; //error: zeroth root is indeterminate!
    if (n == 1) return a;
    tp = iPow(v,n);
    while (tp < a) {    // first power of two such that v**n >= a
        v <<= 1;
        tp = iPow(v,n);
    }
    if (tp == a) return v;  // answer is a power of two
    v >>= 1;
    bit = v >> 1;
    tp = iPow(v, n);    // v is highest power of two such that v**n < a
    while (a > tp) {
        v += bit;       // add bit to value
        t = iPow(v, n);
        if (t > a) v -= bit;    // did we add too much?
        else tp = t;
        if ( (bit >>= 1) == 0) break;
    }
    return v;   // closest integer such that v**n <= a
}


uint32_t SquareRootRounded(uint32_t a_nInput)
{
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type


    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }

    /* Do arithmetic rounding to nearest integer */
    if (op > res)
    {
        res++;
    }

    return res;
}

int ftbl[33]={0,1,1,2,2,4,5,8,11,16,22,32,45,64,90,128,181,256,362,512,724,1024,1448,2048,2896,4096,5792,8192,11585,16384,23170,32768,46340};
int ftbl2[32]={ 32768,33276,33776,34269,34755,35235,35708,36174,36635,37090,37540,37984,38423,38858,39287,39712,40132,40548,40960,41367,41771,42170,42566,42959,43347,43733,44115,44493,44869,45241,45611,45977};

int fisqrt(int val)
{
    int cnt=0;
    int t=val;
    while (t) {cnt++;t>>=1;}
    if (6>=cnt)    t=(val<<(6-cnt));
    else           t=(val>>(cnt-6));

    return (ftbl[cnt]*ftbl2[t&31])>>15;
}