#ifndef LGK_CRC32_H
#define LGK_CRC32_H

#include <stdint.h>

#define LGK_CRC32_TABLE_SIZE 256

void lgk_crc32_table_init(uint_fast32_t table[LGK_CRC32_TABLE_SIZE]);
uint_fast32_t lgk_crc32(const void *buf, unsigned len);
uint_fast32_t lgk_crc32_table(const void *buf, unsigned len, const uint_fast32_t table[LGK_CRC32_TABLE_SIZE]);

#endif
