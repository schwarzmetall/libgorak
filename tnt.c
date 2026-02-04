#include <lgk_tnt.h>

static unsigned is_path_separator(char c)
{
    #ifdef BUILD_HOST_WIN32
    if(c == ':') return 1;
    if(c == '\\') return 1;
    #endif
    if(c == '/') return 1;
    return 0;
}

const char *trace_strip_path(const char *path)
{
    const char *file = path;
    while(*path) if(is_path_separator(*(path++))) file = path;
    return file;
}
