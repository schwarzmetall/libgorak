#ifndef LGK_ATOMIC_BITMAP_H
#define LGK_ATOMIC_BITMAP_H

#include <stdint.h>
#include <stdatomic.h>
#include <lgk/util.h>
#include <lgk/tnt.h>
#include <lgk/bitmap.h>

// Atomic bitmap, lock-free where the target allows, for concurrent flag sets. Same macro-family
// shape as <lgk/bitmap.h>, which this header includes for the shared sizing and tail-mask macros.
//
// A translation unit that never expands an atomic macro emits no atomic code and no libatomic
// references, so including <lgk/bitmap.h> alone stays usable on a target that defines
// __STDC_NO_ATOMICS__. This header does not test that macro: on such an implementation the build
// already fails at the <stdatomic.h> include or the first _Atomic use, and an #error would only
// improve the message.
//
//      ATOMIC_BITMAP_STRUCT(my_flags, _Atomic(uint32_t));
//      static ATOMIC_BITMAP_INIT(my_flags)
//      static ATOMIC_BITMAP_GET_EXPLICIT(my_flags)
//      static ATOMIC_BITMAP_GET(my_flags)
//      static ATOMIC_BITMAP_SET_EXPLICIT(my_flags)
//      static ATOMIC_BITMAP_SET(my_flags)
//
// EXPANSION ORDER: the plain get / set delegate to their _explicit counterparts with
// memory_order_seq_cst, exactly as <stdatomic.h> defines atomic_load in terms of
// atomic_load_explicit. Expand ATOMIC_BITMAP_GET_EXPLICIT before ATOMIC_BITMAP_GET and
// ATOMIC_BITMAP_SET_EXPLICIT before ATOMIC_BITMAP_SET.
//
// There is no is_lock_free operation. With no suggested word type a compile-time macro would have
// nothing to describe, and the caller owns the storage (no allocator), so it can call
// atomic_is_lock_free() on that storage directly - the authoritative per-object answer, and what
// resolves an ATOMIC_*_LOCK_FREE value of 1. A wrapper would only add a NULL / empty-map error
// case the direct call does not have. See <lgk/atomic_widest_lockfree.h> for picking a word type.
//
// TYPE INVARIANT: as for the plain bitmap, bits at indices >= size are always zero.

// init mode. Every mode clears the bits above size; none of them delegates that to the caller.
//
// INIT_* vs STORE_*: atomic objects with automatic or allocated storage duration require
// atomic_init before any other access - using them otherwise is undefined. Objects with static
// storage duration do not. Since there is no allocating constructor, the library cannot tell which
// kind it was handed, so this is a contract on the caller:
//
//  - Storage with automatic or allocated duration, INCLUDING malloc'ed and calloc'ed memory (zeroed
//    bytes are not an atomic_init), must use an INIT_* mode.
//  - ADOPT and STORE_* are only for storage that is already initialized: static storage, or storage
//    a previous INIT_* ran over. Using them on fresh storage is undefined.
enum lgk_atomic_bitmap_init
{
    LGK_ATOMIC_BITMAP_INIT_CLEAR,  // atomic_init every word, data bits 0
    LGK_ATOMIC_BITMAP_INIT_SET,    // atomic_init every word, data bits 1
    LGK_ATOMIC_BITMAP_STORE_CLEAR, // atomic_store 0   (already-initialized storage)
    LGK_ATOMIC_BITMAP_STORE_SET,   // atomic_store all (already-initialized storage)
    LGK_ATOMIC_BITMAP_ADOPT,       // data bits unchanged; storage already initialized
};

// Declares struct <name> over a map of <word>, which must be an _Atomic of a standard unsigned
// integer type, e.g. ATOMIC_BITMAP_STRUCT(my_flags, _Atomic(uint32_t)).
//
// The parameter is the _Atomic element type, not the base type, because C (6.2.5) allows _Atomic T
// to differ from T in size, representation and alignment. Taking the atomic type makes the
// parameter exactly what the map points to and what the caller must declare its storage as; an
// instantiation that spelled only T would invite `T buf[n]`.
//
// Passing a plain (non-atomic) type is rejected by the first assert. That check is not redundant:
// GCC 14 accepts atomic_fetch_or / atomic_load on a plain unsigned * with no diagnostic even under
// -std=c23 -pedantic -Wall -Wextra, so the mistake would otherwise silently produce a non-atomic
// map. const- and volatile-qualified atomics are rejected too - a const map cannot be set, and
// volatile adds nothing here.
//
// As for the plain family there is no default word type; see BITMAP_STRUCT for the word-choice
// guidance, and <lgk/atomic_widest_lockfree.h> for the widest always-lock-free type if the target
// has one.
#define ATOMIC_BITMAP_STRUCT(name, word)\
    struct name\
    {\
        static_assert(IS_ATOMIC_TYPE(word), "atomic bitmap word must be an _Atomic type");\
        static_assert(UWIDTH(word), "atomic bitmap word must be _Atomic of a standard unsigned integer type");\
        word *map;\
        unsigned size;\
    }

// The non-atomic base of the map element, for arithmetic on loaded/stored values. BITMAP_WORD
// yields the _Atomic element type; BITMAP_WORD_BITS and BITMAP_NWORDS work unchanged on an atomic
// instantiation, and BITMAP_NBYTES correctly uses sizeof the _Atomic type as the stride.
#define ATOMIC_BITMAP_WORD_BASE(name) typeof_unqual(*(struct name){}.map)

#define ATOMIC_BITMAP_INIT_HEADER(name) int_fast8_t name##_init(struct name *bitmap, typeof(bitmap->map) map, unsigned size, enum lgk_atomic_bitmap_init mode)
// Attaches bitmap to map, which must hold at least BITMAP_NWORDS(name, size) words and must be
// declared as the instantiated _Atomic element type.
// map may be NULL only when size is 0.
// See enum lgk_atomic_bitmap_init for which modes are legal for which storage duration; picking a
// STORE_* or ADOPT mode for storage that was never atomic_init'ed is undefined.
// Returns 0, or -1 on invalid parameters.
#define ATOMIC_BITMAP_INIT(name)\
    ATOMIC_BITMAP_INIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        if(size) TRAPVNULL(map);\
        TRAPVXXOOR(mode, LGK_ATOMIC_BITMAP_INIT_CLEAR, LGK_ATOMIC_BITMAP_ADOPT, init_clear, adopt, "i");\
        bitmap->map = map;\
        bitmap->size = size;\
        if(size)\
        {\
            const unsigned bits = BITMAP_WORD_BITS(name);\
            const unsigned nwords = INTDIVCEIL(size, bits);\
            const unsigned rem = size % bits;\
            const ATOMIC_BITMAP_WORD_BASE(name) ones = (ATOMIC_BITMAP_WORD_BASE(name))~(ATOMIC_BITMAP_WORD_BASE(name))0;\
            /* value of the final word under a SET mode: masked when partial, all ones when full */\
            const ATOMIC_BITMAP_WORD_BASE(name) tail_set = rem ? BITMAP_TAIL_MASK(ATOMIC_BITMAP_WORD_BASE(name), size) : ones;\
            switch(mode)\
            {\
            case LGK_ATOMIC_BITMAP_INIT_CLEAR:\
                for(unsigned i_word = 0; i_word < nwords; i_word++) atomic_init(map+i_word, 0);\
                break;\
            case LGK_ATOMIC_BITMAP_INIT_SET:\
                for(unsigned i_word = 0; i_word + 1 < nwords; i_word++) atomic_init(map+i_word, ones);\
                atomic_init(map+nwords-1, tail_set);\
                break;\
            case LGK_ATOMIC_BITMAP_STORE_CLEAR:\
                for(unsigned i_word = 0; i_word < nwords; i_word++) atomic_store(map+i_word, 0);\
                break;\
            case LGK_ATOMIC_BITMAP_STORE_SET:\
                for(unsigned i_word = 0; i_word + 1 < nwords; i_word++) atomic_store(map+i_word, ones);\
                atomic_store(map+nwords-1, tail_set);\
                break;\
            case LGK_ATOMIC_BITMAP_ADOPT:\
                /* one RMW on the final word, safe even if other threads are already using the */\
                /* array; nothing to do when the map ends on a word boundary */\
                if(rem) atomic_fetch_and(map+nwords-1, tail_set);\
                break;\
            }\
        }\
        return 0;\
    trap_mode_oor_init_clear_adopt:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define ATOMIC_BITMAP_GET_EXPLICIT_HEADER(name) int_fast8_t name##_get_explicit(const struct name *bitmap, unsigned i, memory_order order)
// Returns the bit at i (0 or 1), or -1 on invalid parameters, including i >= size.
// order must not be memory_order_release or memory_order_acq_rel: atomic_load_explicit with either
// is undefined, so both are rejected like any other invalid parameter.
// HAZARD: `if(my_flags_get(bm, i))` treats the error as "set". Compare < 0 explicitly first.
#define ATOMIC_BITMAP_GET_EXPLICIT(name)\
    ATOMIC_BITMAP_GET_EXPLICIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        TRAPVXEQ(order, memory_order_release, release, "i");\
        TRAPVXEQ(order, memory_order_acq_rel, acq_rel, "i");\
        const unsigned bits = BITMAP_WORD_BITS(name);\
        const ATOMIC_BITMAP_WORD_BASE(name) mask = (ATOMIC_BITMAP_WORD_BASE(name))1 << (i % bits);\
        return ((atomic_load_explicit(bitmap->map + (i / bits), order) & mask) != 0);\
    trap_order_eq_acq_rel:\
    trap_order_eq_release:\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define ATOMIC_BITMAP_GET_HEADER(name) int_fast8_t name##_get(const struct name *bitmap, unsigned i)
// memory_order_seq_cst get. Requires ATOMIC_BITMAP_GET_EXPLICIT(name) to be expanded first.
#define ATOMIC_BITMAP_GET(name)\
    ATOMIC_BITMAP_GET_HEADER(name)\
    {\
        return name##_get_explicit(bitmap, i, memory_order_seq_cst);\
    }

#define ATOMIC_BITMAP_SET_EXPLICIT_HEADER(name) int_fast8_t name##_set_explicit(struct name *bitmap, unsigned i, int_fast8_t setval, memory_order order)
// Writes bit i: setval == 0 clears, anything else sets.
// Returns (setval != 0) on success, or -1 on invalid parameters, including i >= size.
// Every memory order is valid for a read-modify-write, so order is not restricted here.
// HAZARD: `if(my_flags_set(bm, i, v))` is true both when the bit was written as 1 and when the call
// failed. Compare < 0 explicitly before treating the return as the value written.
//
// Uses fetch_or / fetch_and, never a load-modify-store: the latter is not atomic and would lose
// concurrent updates to *other* bits sharing the word.
//
// Does not report the previous bit. The RMW yields the previous *word*, so isolating one bit would
// cost an extra AND and shift on every call whether or not the caller wants it. If that is ever
// needed, it belongs in a separately named operation analogous to atomic_flag_test_and_set().
#define ATOMIC_BITMAP_SET_EXPLICIT(name)\
    ATOMIC_BITMAP_SET_EXPLICIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        const unsigned bits = BITMAP_WORD_BITS(name);\
        const ATOMIC_BITMAP_WORD_BASE(name) mask = (ATOMIC_BITMAP_WORD_BASE(name))1 << (i % bits);\
        if(setval) atomic_fetch_or_explicit(bitmap->map + (i / bits), mask, order);\
        else atomic_fetch_and_explicit(bitmap->map + (i / bits), (ATOMIC_BITMAP_WORD_BASE(name))~mask, order);\
        return (setval != 0);\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define ATOMIC_BITMAP_SET_HEADER(name) int_fast8_t name##_set(struct name *bitmap, unsigned i, int_fast8_t setval)
// memory_order_seq_cst set. Requires ATOMIC_BITMAP_SET_EXPLICIT(name) to be expanded first.
#define ATOMIC_BITMAP_SET(name)\
    ATOMIC_BITMAP_SET_HEADER(name)\
    {\
        return name##_set_explicit(bitmap, i, setval, memory_order_seq_cst);\
    }

#endif
