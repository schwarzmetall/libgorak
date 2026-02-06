/* Write until exactly count bytes or error/timeout. Uses poll() to avoid blocking. A return value != count indicates an error or timeout.
   On error, *err is set to -1 and errno contains the value set by the failing poll() or write() call. On success or timeout, *err is set to 0.*/
#include <stdint.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <lgk_tnt.h>
#include <lgk_fd.h>

constexpr short POLLERRMASK = POLLERR | POLLNVAL;
#ifdef _XOPEN_SOURCE
constexpr short POLLWRITEMASK = POLLOUT | POLLWRNORM;
#else
constexpr short POLLWRITEMASK = POLLOUT;
#endif

size_t fd_write_loop(int fd, const void *buf, size_t count, int timeout_ms, int_fast8_t *err)
{
    const uint8_t *restrict p_buf = buf;
    if(!count) return 0;
    size_t nleft = count;
    while(nleft)
    {
        struct pollfd fds = { fd, POLLWRITEMASK, 0 };
        int status_poll = poll(&fds, 1, timeout_ms);
        TRAPFE((status_poll<0)||(fds.revents&POLLERRMASK), poll);
        if(!status_poll || (fds.revents & POLLHUP)) break;
        ssize_t nwritten = write(fd, p_buf, nleft);
        TRAPFE(nwritten<0, write);
        nleft -= nwritten;
        p_buf += nwritten;
    }
    if(err) *err = 0;
    return count-nleft;
trap_write:
trap_poll:
    if(err) *err = -1;
    return count-nleft;
}
