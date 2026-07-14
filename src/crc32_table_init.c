#include <lgk/crc32.h>
#include <lgk/tnt.h>

void lgk_crc32_table_init(uint_fast32_t table[LGK_CRC32_TABLE_SIZE])
{
    TRAPVNULL(table);
    for(unsigned i=0; i<LGK_CRC32_TABLE_SIZE; i++)
    {
        uint32_t r = i;
        for(unsigned j=0; j<8; j++) r = (r>>1) ^ (0xEDB88320u & -(r & 1u));
        table[i] = r;
    }
    return;
trap_table_null:
    return;
}
