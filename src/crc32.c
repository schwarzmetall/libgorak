#include <lgk/crc32.h>
#include <lgk/tnt.h>
#include <stdint.h>

uint_fast32_t lgk_crc32(const void *buf, unsigned len)
{
    TRAPVNULL(buf);
    const uint8_t *p = buf;
    uint_fast32_t crc = 0xFFFFFFFFu;
    for(unsigned i = 0; i < len; i++)
    {
        crc ^= p[i];
        for(unsigned j = 0; j < 8; j++) crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1u));
    }
    return crc ^ 0xFFFFFFFFu;
trap_buf_null:
    return 0;
}
