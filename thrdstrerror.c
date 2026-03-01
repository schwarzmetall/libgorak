#include <stdio.h>
#include <lgk_threads.h>

#define RETURN_STR_ON_MATCH(var, val) if(var==val) return #val

const char *lgk_thrdstrerror(int status)
{
    static char buf_undef[32] = {};
    RETURN_STR_ON_MATCH(status, thrd_success);
    RETURN_STR_ON_MATCH(status, thrd_timedout);
    RETURN_STR_ON_MATCH(status, thrd_busy);
    RETURN_STR_ON_MATCH(status, thrd_nomem);
    RETURN_STR_ON_MATCH(status, thrd_error);
    sprintf(buf_undef, "%i", status);
    return buf_undef;
}
