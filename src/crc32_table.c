#include <lgk/crc32.h>
#include <lgk/tnt.h>
#include <stdint.h>

uint_fast32_t lgk_crc32_table(const void *buf, unsigned len, const uint_fast32_t table[LGK_CRC32_TABLE_SIZE])
{
    TRAPVNULL(buf);
    TRAPVNULL(table);
    const uint8_t *p = buf;
    uint_fast32_t crc = 0xFFFFFFFFu;
    for(unsigned i = 0; i < len; i++) crc = (crc >> 8) ^ table[(crc ^ p[i]) & 0xFFu];
    return crc ^ 0xFFFFFFFFu;
trap_buf_null:
    return 0;
trap_table_null:
    return 0;
}
