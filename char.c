#include <stdint.h>
#include <lgk_char.h>

int_fast8_t char_isctrl(char c)
{
    return (unsigned char)c < 33;
}

int_fast8_t char_iswhitespace(char c)
{
    return (c == ' ') || (c == '\t');
}

int_fast8_t char_iseol(char c)
{
    return((c == '\n') || (c == '\r'));
}

unsigned char_skip_to(char c, const char *str, unsigned n)
{
    for(unsigned i=0; i<n; i++) if(str[i]==c) return i;
    return n;
}

unsigned char_skip_to_eol(const char *str, unsigned n)
{
    for(unsigned i=0; i<n; i++) if(char_iseol(str[i])) return i;
    return n;
}

unsigned char_skip_until(char_match *match, const char *str, unsigned n)
{
    for(unsigned i=0; i<n; i++) if(match(str[i])) return i;
    return n;
}

// returns number of leading whitespace characters (number of characters to be trimmed)
unsigned char_trim_front(const char *str, unsigned n)
{
    for(unsigned i=0; i<n; i++) if(!char_iswhitespace(str[i])) return i;
    return n;
}

// returns number of characters before trailing whitespace (number of characters to leave in string)
unsigned char_trim_back(const char *str, unsigned n)
{
    for(unsigned i=n; i; i--) if(!char_iswhitespace(str[i-1])) return i;
    return 0;
}
