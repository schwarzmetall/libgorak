#ifndef LGK_FD_H
#define LGK_FD_H

#include <stdint.h>

/* Read/write exactly count bytes, using poll() so each wait is bounded by timeout_ms.
 * timeout_ms: max ms to wait for fd to become readable/writable per poll; <0 = wait forever.
 * Return number of bytes read/written; if less than count, treat as error or timeout.
 * On error, *err is set to -1 and errno is set by the failing call; on success or timeout, *err is set to 0. */
size_t fd_read_loop(int fd, void *buf, size_t count, int timeout_ms, int_fast8_t *err);
size_t fd_write_loop(int fd, const void *buf, size_t count, int timeout_ms, int_fast8_t *err);

/* Single read/write after poll with timeout. Return value from read()/write() on success (or 0 on timeout); -1 on poll error. *pollerr: 0 = no poll error, -1 = poll error. */
ssize_t fd_read_timed(int fd, void *buf, size_t count, int timeout_ms, int_fast8_t *pollerr);
ssize_t fd_write_timed(int fd, const void *buf, size_t count, int timeout_ms, int_fast8_t *pollerr);

#endif
