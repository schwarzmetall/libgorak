#include <lgk/tnt.h>
#include <lgk/util.h>
#ifdef USE_FPU
    #include <math.h>
#endif

unsigned digits(unsigned value, unsigned base)
{
#ifdef USE_FPU // TODO
    return (unsigned)(log(value) / log(base)) + 1;
#else
    unsigned n=0;
    for(n=0; value; n++) value /= base;
    return n;
#endif
}
