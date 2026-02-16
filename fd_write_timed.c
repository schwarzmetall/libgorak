/* Single write after poll with timeout. Uses poll() then one write(). */
#include <stdint.h>
#include <poll.h>
#include <unistd.h>
#include <lgk_tnt.h>
#include <lgk_fd.h>

ssize_t fd_write_timed(int fd, const void *buf, size_t count, int timeout_ms, int_fast8_t *pollerr)
{
    if(!count) return 0;
    struct pollfd fds = { fd, POLLOUT, 0 };
    int status_poll = poll(&fds, 1, timeout_ms);
    TRAPFE((status_poll<0)||(fds.revents&(POLLERR|POLLNVAL)), poll);
    if(pollerr) *pollerr = 0;
    if(!status_poll) return 0;
    if(fds.revents & POLLHUP) return 0;
    ssize_t nwritten = write(fd, buf, count);
    TRAPFE(nwritten<0, write);
    return nwritten;
trap_write:
trap_poll:
    if(pollerr) *pollerr = -1;
    return -1;
}
