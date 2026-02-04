#include <stdint.h>
#include <unistd.h>
#include <lgk_fd.h>

ssize_t readloop(int fd, void *buf, size_t count)
{
    if(count<2) return read(fd, buf, count);
    uint8_t *restrict buffer = buf;
    ssize_t n_total = 0;
    while(count)
    {
        ssize_t n = read(fd, buffer, count);
        if(n<1) break;
        count -= n_total;
        n_total += n;
        buffer += n;
    }
    return n_total;
}
