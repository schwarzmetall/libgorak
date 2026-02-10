#ifndef LGK_FD_H
#define LGK_FD_H

#include <stdint.h>

/* Read/write exactly count bytes, using poll() so each wait is bounded by timeout_ms.
 * timeout_ms: max ms to wait for fd to become readable/writable per poll; <0 = wait forever.
 * Return number of bytes read/written; if less than count, treat as error or timeout.
 * On error, *err is set to -1 and errno is set by the failing call; on success or timeout, *err is set to 0. */
size_t readloop(int fd, void *buf, size_t count, int timeout_ms, int_least8_t *err);
size_t writeloop(int fd, const void *buf, size_t count, int timeout_ms, int_least8_t *err);

#endif
