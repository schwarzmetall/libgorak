#ifndef LGK_BITMAP_H
#define LGK_BITMAP_H

#include <stdint.h>

typedef uint_fast8_t lgk_bitmap_t;

struct lgk_bitmap
{
    // TODO - can we rely in this to be efficient?
    lgk_bitmap_t *map;
    unsigned size;
};

int_fast8_t lgk_bitmap_init(struct lgk_bitmap *bitmap, lgk_bitmap_t *map, unsigned size, int_fast8_t setval);
int_fast8_t lgk_bitmap_get(const struct lgk_bitmap *bitmap, unsigned i);
int_fast8_t lgk_bitmap_set(struct lgk_bitmap *bitmap, unsigned i, int_fast8_t setval);
struct lgk_bitmap *lgk_bitmap_alloc(unsigned size, int_fast8_t setval);
void lgk_bitmap_free(struct lgk_bitmap *lgk_bitmap_initmap);

#endif
