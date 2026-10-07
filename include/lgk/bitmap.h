#ifndef LGK_BITMAP_H
#define LGK_BITMAP_H

#include <stdint.h>
#include <string.h>
#include <lgk/util.h>
#include <lgk/tnt.h>

// Non-atomic bitmap, for use where atomicity is NOT a requirement (single-threaded use, or
// externally serialized access). For concurrent flag sets see <lgk/atomic_bitmap.h>.
//
// This header defines no struct, declares no function and suggests no word type; it is a macro
// family in the same shape as <lgk/heap.h>. Each *_HEADER macro is the prototype; the unsuffixed
// macro is that prototype plus the body and begins at the return type, so the instantiator chooses
// linkage:
//
//      BITMAP_STRUCT(my_bits, uint32_t);
//      static BITMAP_INIT(my_bits)
//      static BITMAP_GET(my_bits)
//      static BITMAP_SET(my_bits)
//
// Instantiated names can collide like those of any other libgorak macro family; choosing names and
// linkage is the instantiator's job.
//
// The bitmap struct is a descriptor over storage it never owns. There is no allocating constructor
// and no free: the caller provides the word array (static, automatic, heap, arena, or embedded in
// its own struct) and passes it to init. BITMAP_NWORDS / BITMAP_NBYTES give everything an
// allocator would compute.
//
// TYPE INVARIANT: bits at indices >= size are always zero. Every init mode establishes this, and
// set preserves it (an out-of-range index is an error, never a write). Future scanning/counting
// relies on it to run over whole words with no special case for the final, partial word.

// init mode. Every mode clears the bits above size; none of them delegates that to the caller.
enum lgk_bitmap_init
{
    LGK_BITMAP_CLEAR,  // all data bits 0
    LGK_BITMAP_SET,    // all data bits 1
    LGK_BITMAP_ADOPT,  // data bits unchanged; storage shall already be initialized
};

// Declares struct <name> over a map of <word>.
//
// <word> must be a standard unsigned integer type; anything else is a compile error (see UWIDTH in
// util.h). Typedefs of standard unsigned types are fine (uintN_t, uintptr_t, uint_fast8_t, size_t,
// ...). Types with padding bits are accepted: the bit count is the value width, and padding bits
// are never addressed by an index.
//
// There is deliberately no default word type, and passing a type is the only way to choose the
// word: C cannot name the native register width (the missing (u)int_native_t), and every available
// proxy is wrong on some common target - uintptr_t is too wide on 8-bit parts with 16-bit pointers
// and too narrow on x32, unsigned int is 32 bits on most 64-bit targets, and glibc freezes
// uint_fast8_t at 8 bits. A default that cannot know the right answer would only invite relying on
// it.
//
// Choosing a word:
//  - Scan cost is word count. There is no per-word synchronization in the loop, so the iteration
//    count for a future find-first-set / popcount is size divided by the word width: 8-bit -> 64-bit
//    is 8x fewer loop trips.
//  - Allocation granularity is at most one word minus one bit per bitmap, so small bitmaps on
//    memory-constrained targets favour narrow words.
#define BITMAP_STRUCT(name, word)\
    struct name\
    {\
        static_assert(UWIDTH(word), "bitmap word must be a standard unsigned integer type");\
        word *map;\
        unsigned size;\
    }

// Element type of the map. For an atomic instantiation this is the _Atomic type.
#define BITMAP_WORD(name) STYPEOF_DEREF(name, map)
// Bits per word: the value width of the non-atomic base (UWIDTH strips _Atomic).
#define BITMAP_WORD_BITS(name) UWIDTH(STYPEOF_DEREF(name, map))
// Words needed to hold nbits bits.
#define BITMAP_NWORDS(name, nbits) INTDIVCEIL((nbits), BITMAP_WORD_BITS(name))
// Bytes the caller must provide for nbits bits. The stride is sizeof the *element* type, because
// sizeof(_Atomic T) may exceed sizeof(T) and a padded T occupies more bits than it has value bits.
#define BITMAP_NBYTES(name, nbits) (BITMAP_NWORDS(name, nbits) * sizeof(BITMAP_WORD(name)))

// All four are integer constant expressions, so a static backing array can be sized for any
// instantiation:
//      static BITMAP_WORD(my_bits) storage[BITMAP_NWORDS(my_bits, 70)];
//      BITMAP_WORD(my_bits) *heap_storage = malloc(BITMAP_NBYTES(my_bits, n));
// They are not usable in #if.

// Mask of the data bits in the final, partial word of an nbits-bit map, for the (non-atomic) word
// type <word>. Computes the mask only; applying it is the caller's job (plain operations use &=,
// atomic ones atomic_fetch_and).
//
// Valid ONLY when (nbits % UWIDTH(word)) != 0. With a zero remainder there is no partial word and
// this yields an all-zero mask, which would clear a full and entirely valid word - callers must
// skip the fixup in that case. Inside the partial-word branch the shift amount is in 1 .. bits-1,
// so the shift is defined.
#define BITMAP_TAIL_MASK(word, nbits) ((word)(((word)1 << ((nbits) % UWIDTH(word))) - 1))

#define BITMAP_INIT_HEADER(name) int_fast8_t name##_init(struct name *bitmap, typeof(bitmap->map) map, unsigned size, enum lgk_bitmap_init mode)
// Attaches bitmap to map, which must hold at least BITMAP_NWORDS(name, size) words.
// map may be NULL only when size is 0.
// LGK_BITMAP_ADOPT requires map to be initialized; adopting indeterminate storage is the caller's
// error. Masking an indeterminate word would not rescue it - per the committee response to DR 451,
// operations on indeterminate values yield indeterminate results.
// Returns 0, or -1 on invalid parameters.
#define BITMAP_INIT(name)\
    BITMAP_INIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        if(size) TRAPVNULL(map);\
        TRAPVXXOOR(mode, LGK_BITMAP_CLEAR, LGK_BITMAP_ADOPT, clear, adopt, "i");\
        bitmap->map = map;\
        bitmap->size = size;\
        if(size)\
        {\
            const unsigned bits = BITMAP_WORD_BITS(name);\
            const unsigned nwords = INTDIVCEIL(size, bits);\
            const unsigned rem = size % bits;\
            if(mode == LGK_BITMAP_CLEAR)\
            {\
                /* all-bits-zero is a representation of 0 for every integer type, padding included */\
                memset(map, 0, (size_t)nwords * sizeof(*map));\
            }\
            else\
            {\
                /* LGK_BITMAP_SET assigns ~(word)0 rather than memset(0xFF), which would also set */\
                /* any padding bits and could produce a non-value representation */\
                if(mode == LGK_BITMAP_SET) for(unsigned i_word = 0; i_word < nwords; i_word++) map[i_word] = (typeof(*map))~(typeof(*map))0;\
                if(rem) map[nwords-1] &= BITMAP_TAIL_MASK(typeof(*map), size);\
            }\
        }\
        return 0;\
    trap_mode_oor_clear_adopt:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define BITMAP_GET_HEADER(name) int_fast8_t name##_get(const struct name *bitmap, unsigned i)
// Returns the bit at i (0 or 1), or -1 on invalid parameters, including i >= size.
// HAZARD: `if(my_bits_get(bm, i))` treats the error as "set". Compare < 0 explicitly first.
#define BITMAP_GET(name)\
    BITMAP_GET_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        const unsigned bits = BITMAP_WORD_BITS(name);\
        const BITMAP_WORD(name) mask = (BITMAP_WORD(name))1 << (i % bits);\
        return ((bitmap->map[i / bits] & mask) != 0);\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define BITMAP_SET_HEADER(name) int_fast8_t name##_set(struct name *bitmap, unsigned i, int_fast8_t setval)
// Writes bit i: setval == 0 clears, anything else sets.
// Returns (setval != 0) on success, or -1 on invalid parameters, including i >= size.
// HAZARD: `if(my_bits_set(bm, i, v))` is true both when the bit was written as 1 and when the call
// failed. Compare < 0 explicitly before treating the return as the value written.
// Does not report the previous bit; see the atomic family's notes for why.
#define BITMAP_SET(name)\
    BITMAP_SET_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        const unsigned bits = BITMAP_WORD_BITS(name);\
        const BITMAP_WORD(name) mask = (BITMAP_WORD(name))1 << (i % bits);\
        if(setval) bitmap->map[i / bits] |= mask;\
        else bitmap->map[i / bits] &= (BITMAP_WORD(name))~mask;\
        return (setval != 0);\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#endif
