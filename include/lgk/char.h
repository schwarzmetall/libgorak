#ifndef LGK_CHAR_H
#define LGK_CHAR_H

#include <stdint.h>

typedef int_fast8_t char_match(char c);

int_fast8_t char_isctrl(char c);
int_fast8_t char_iswhitespace(char c);
int_fast8_t char_iseol(char c);

unsigned char_skip_to(char c, const char *str, unsigned n);
unsigned char_skip_to_eol(const char *str, unsigned n);
unsigned char_skip_matching(char_match *match, const char *str, unsigned n);

unsigned char_trim_front(const char *str, unsigned n);
unsigned char_trim_back(const char *str, unsigned n);

#endif
