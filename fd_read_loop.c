/* Read until exactly count bytes or error/timeout. Uses poll() to avoid blocking. A return value != count indicates an error or timeout.
   On error, *err is set to -1 and errno contains the value set by the failing poll() or read() call. On success or timeout, *err is set to 0.*/
#include <stdint.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <lgk_tnt.h>
#include <lgk_fd.h>

size_t fd_read_loop(int fd, void *buf, size_t count, int timeout_ms, int_fast8_t *err)
{
    uint8_t *restrict p_buf = buf;
    if(!count) return 0;
    size_t nleft = count;
    while(nleft)
    {
        struct pollfd fds = { fd, POLLIN, 0 };
        int status_poll = poll(&fds, 1, timeout_ms);
        TRAPFE((status_poll<0)||(fds.revents&(POLLERR|POLLNVAL)), poll);
        if(!status_poll) break;
        ssize_t nread = read(fd, p_buf, nleft);
        TRAPFE(nread<0, read);
        if(!nread) break;
        nleft -= nread;
        p_buf += nread;
    }
    if(err) *err = 0;
    return count-nleft;
trap_read:
trap_poll:
    if(err) *err = -1;
    return count-nleft;
}
