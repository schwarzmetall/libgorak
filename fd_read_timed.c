/* Single read after poll with timeout. Uses poll() then one read(). */
#include <stdint.h>
#include <poll.h>
#include <unistd.h>
#include <lgk_tnt.h>
#include <lgk_fd.h>

ssize_t fd_read_timed(int fd, void *buf, size_t count, int timeout_ms, int_fast8_t *pollerr)
{
    if(!count) return 0;
    struct pollfd fds = { fd, POLLIN, 0 };
    int status_poll = poll(&fds, 1, timeout_ms);
    TRAPFE((status_poll<0)||(fds.revents&(POLLERR|POLLNVAL)), poll);
    if(pollerr) *pollerr = 0;
    if(!status_poll) return 0;
    ssize_t nread = read(fd, buf, count);
    TRAPFE(nread<0, read);
    return nread;
trap_read:
trap_poll:
    if(pollerr) *pollerr = -1;
    return -1;
}
