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
