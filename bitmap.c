#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <lgk_util.h>
#include <lgk_tnt.h>
#include <lgk_bitmap.h>

// setval: 0 -> set all to zero; >0 -> set all to 1; <0 -> don't touch
int_fast8_t lgk_bitmap_init(struct lgk_bitmap *bitmap, lgk_bitmap_t *map, unsigned size, int_fast8_t setval)
{
    TRAPNULL(bitmap);
    if(size) TRAPNULL(map);
    bitmap->map = map;
    bitmap->size = size;
    if(setval<0) return 0;
    if(setval) setval = -1;
    if(size) memset(map, setval, INTDIVCEIL(size,8));
    return 0;
trap_map_null:
trap_bitmap_null:
    return -1;
}

int_fast8_t lgk_bitmap_get(const struct lgk_bitmap *bitmap, unsigned i)
{
    TRAPNULL(bitmap);
    TRAPNULL_L(bitmap->map, size);
    TRAPVEQ(bitmap->size, 0, size_zero);
    TRAP(i >= bitmap->size, bounds, "index %u >= size %u", i, bitmap->size);
    unsigned i_word = i / BITSIZE(bitmap->map[0]);
    lgk_bitmap_t mask = 1 << (i % BITSIZE(bitmap->map[0]));
    return ((bitmap->map[i_word] & mask) != 0);
trap_bounds:
trap_size_zero:
trap_size_null:
trap_bitmap_null:
    return -1;
}

int_fast8_t lgk_bitmap_set(struct lgk_bitmap *bitmap, unsigned i, int_fast8_t setval)
{
    TRAPNULL(bitmap);
    TRAPNULL_L(bitmap->map, size);
    TRAPVEQ(bitmap->size, 0, size_zero);
    TRAP(i >= bitmap->size, bounds, "index %u >= size %u", i, bitmap->size);
    unsigned i_word = i / BITSIZE(bitmap->map[0]);
    lgk_bitmap_t mask = 1 << (i % BITSIZE(bitmap->map[0]));
    bitmap->map[i_word] = setval ? (bitmap->map[i_word] | mask) : ( bitmap->map[i_word] & ~mask);
    return (setval != 0);
trap_bounds:
trap_size_zero:
trap_size_null:
trap_bitmap_null:
    return -1;
}

struct lgk_bitmap *lgk_bitmap_alloc(unsigned size, int_fast8_t setval)
{
    struct lgk_bitmap *bitmap = malloc(sizeof(struct lgk_bitmap));
    TRAPFES(!bitmap, malloc, struct);
    lgk_bitmap_t *map = NULL;
    if(size)
    {
        map =  malloc(INTDIVCEIL(size,8));
        TRAPFES(!map, malloc, map);
    }
    int_fast8_t status = lgk_bitmap_init(bitmap, map, size, setval);
    TRAPF(status, lgk_bitmap_init, "%i", status);
    return bitmap;
trap_lgk_bitmap_init:
    free(map);
trap_malloc_map:
    free(bitmap);
trap_malloc_struct:
    return NULL;
}

void lgk_bitmap_free(struct lgk_bitmap *bitmap)
{
    if(!bitmap) return;
    free(bitmap->map);
    free(bitmap);
}

