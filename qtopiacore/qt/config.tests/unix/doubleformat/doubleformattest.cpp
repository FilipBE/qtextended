/*

LE: strings | grep 0123ABCD0123ABCD
BE: strings | grep DCBA3210DCBA3210

LE arm-swaped-dword-order: strings | grep ABCD0123ABCD0123
BE arm-swaped-dword-order: strings | grep 3210DCBA3210DCBA (untested)

tested on x86, arm-le (gp), aix

*/


// equals static char c [] = "0123ABCD0123ABCD\0\0\0\0\0\0\0"
static  double d [] = { 710524581542275055616.0, 710524581542275055616.0};

int main()
{
    // make sure the linker doesn't throw away the arrays
    double *d2 = (double *) d;
    return d[0] == d[1];
}
