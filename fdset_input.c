#include <stdint.h>
#include <threads.h>
#include <unistd.h>
#include <lgk_tnt.h>
#include <lgk_fd.h>
#include <lgk_threads.h>
#include <lgk_fdset_input.h>

constexpr uint8_t THREAD_CMD_RUN = 0;
constexpr uint8_t THREAD_CMD_PAUSE = 1;

constexpr short POLLERRMASK = POLLERR | POLLNVAL;
#ifdef _XOPEN_SOURCE
constexpr short POLLREADMASK = POLLIN | POLLRDNORM;
#else
constexpr short POLLREADMASK = POLLIN;
#endif

typedef void callback_action(struct fdset_input *fi, unsigned fd_index);

static void callback_action_clear_buffer(struct fdset_input *fi, unsigned fd_index)
{
    fi->fd_info_buffer[fd_index].buffer_used = 0;
}

static void callback_action_remove_fd(struct fdset_input *fi, unsigned fd_index)
{
    fi->pollfd_buffer[fd_index+1] = fi->pollfd_buffer[fi->nused--];
    fi->fd_info_buffer[fd_index] = fi->fd_info_buffer[fi->nused];
}

static void callback_exec(struct fdset_input *fi, unsigned fd_index, enum fdset_input_status status)
{
    static callback_action *const callback_action_table[N_FDSET_INPUT_CALLBACK_ACTIONS] =
    {
        NULL,
        callback_action_clear_buffer,
        callback_action_remove_fd
    };
    struct fdset_input_fd_info *fd_info = fi->fd_info_buffer + fd_index;
    struct pollfd *pollfd = fi->pollfd_buffer + fd_index + 1;
    enum fdset_input_callback_action action = fd_info->callback(pollfd->fd, fd_info->buffer, fd_info->buffer_used, status, fd_info->arg);
    TRAP(action>=N_FDSET_INPUT_CALLBACK_ACTIONS, action_invalid, "action==%i", action);
    if(callback_action_table[action]) callback_action_table[action](fi, fd_index);
trap_action_invalid:
}

static int fdset_input_thread(void *data)
{
    uint8_t cmd = THREAD_CMD_RUN;
    struct fdset_input *restrict fi = data;
    int_fast8_t pipe_hup = 0;
    while(!(pipe_hup || fi->close))
    {
        while(cmd == THREAD_CMD_PAUSE)
        {
            /* read() returns when fdset_input_close() closes the write end of the pipe (read returns 0, EOF). */
            ssize_t nread = read(fi->pipefd_read, &cmd, 1);
            TRAPFES(nread != 1, read, pipe_wait);
        }
        int status = mtx_timedlock_ms(&fi->mutex, fi->timeout_ms);
        TRAPF(status!=thrd_success, mtx_timedlock_ms, "%i", status);
        int nevents = poll(fi->pollfd_buffer, fi->nused+1, (int)fi->timeout_ms);
        TRAPFE(nevents<0, poll);
        short pipe_revents = fi->pollfd_buffer[0].revents;
        if(pipe_revents)
        {
            if(pipe_revents & POLLHUP) pipe_hup = -1;
            if(pipe_revents & POLLREADMASK)
            {
                /* Follow-up to poll() that indicated POLLIN; data is available so read should not block. */
                ssize_t nread = read(fi->pipefd_read, &cmd, 1);
                TRAPFES(nread != 1, read, pipe_pollin);
            }
            if(pipe_revents & POLLERRMASK) CRIT("errors reported in pipe_revents: 0x%04hx", pipe_revents);
            if(pipe_revents & ~(POLLERRMASK | POLLREADMASK | POLLHUP)) CRIT("unhandled flags in pipe_revents: 0x%04hx", pipe_revents);
            nevents--;
        }
        for(unsigned i=0; nevents && (i<fi->nused); i++)
        {
            struct pollfd *pollfd = fi->pollfd_buffer+i+1;
            short revents = pollfd->revents;
            if(revents)
            {
                struct fdset_input_fd_info *fd_info = fi->fd_info_buffer + i;
                if(revents & POLLREADMASK)
                {
                    uint8_t *buf_read = ((uint8_t*)fd_info->buffer) + fd_info->buffer_used;
                    /* Follow-up to poll() that indicated POLLIN for this fd; data is available so read should not block. */
                    ssize_t nread = read(pollfd->fd, buf_read, fd_info->buffer_size - fd_info->buffer_used);
                    if(nread > 0) fd_info->buffer_used += nread;
                    if(fd_info->callback) callback_exec(fi, i, nread ? ((nread < 0) ? FDSET_INPUT_STATUS_ERROR_READ : FDSET_INPUT_STATUS_OK) : FDSET_INPUT_STATUS_ZERO_READ);
                }
                if(revents & POLLERRMASK) if(fd_info->callback) callback_exec(fi, i, FDSET_INPUT_STATUS_ERROR_POLL);
                if(revents & ~(POLLERRMASK | POLLREADMASK | POLLHUP)) CRIT("unhandled flags in revents: 0x%04hx", revents);
                nevents--;
            }
        }
        status = mtx_unlock(&fi->mutex);
        TRAPF(status!=thrd_success, mtx_unlock, "%i", status);
    }
    TRAPVEQ(fi->close, 0, close_eq_zero);
    if(close(fi->pipefd_read)) CRITFE(close);
    return thrd_success;
trap_close_eq_zero:
trap_mtx_unlock:
trap_read_pipe_pollin:
trap_poll:
trap_mtx_timedlock_ms:
trap_read_pipe_wait:
    if(close(fi->pipefd_read)) CRITFE(close);
    return thrd_error;
}

int fdset_input_init(struct fdset_input *fi, struct fdset_input_fd_info *fd_info_buffer, struct pollfd *pollfd_buffer, unsigned nmax, int timeout_ms)
{
    TRAPNULL(fi);
    TRAPNULL(fd_info_buffer);
    TRAPNULL(pollfd_buffer);
    TRAP(!nmax, nmax, "nmax==%i", nmax);
    fi->fd_info_buffer = fd_info_buffer;
    fi->pollfd_buffer = pollfd_buffer;
    fi->nmax = nmax;
    fi->nused = 0;
    fi->timeout_ms = timeout_ms;
    fi->close = 0;
    int status = pipe(fi->pipefd);
    TRAPFE(status, pipe);
    pollfd_buffer[0] = (struct pollfd){fi->pipefd_read, POLLIN, 0};
    status = mtx_init(&fi->mutex, (timeout_ms<0)?mtx_plain:mtx_timed);
    TRAPF(status!=thrd_success, mtx_init, "%i", status);
    status = lgk_monitor_init(&fi->thread_mon, (timeout_ms>0));
    TRAPF(status!=thrd_success, lgk_monitor_init, "%i", status);
    status = lgk_thread_create(&fi->thread, fdset_input_thread, fi, &fi->thread_mon);
    TRAPF(status!=thrd_success, lgk_thread_create, "%i", status);
    return thrd_success;
trap_lgk_thread_create:
    if((status=lgk_monitor_destroy(&fi->thread_mon))) CRITF(lgk_monitor_destroy, "%i", status);
trap_lgk_monitor_init:
    mtx_destroy(&fi->mutex);
trap_mtx_init:
    if(close(fi->pipefd_write)) CRITFE(close);
    if(close(fi->pipefd_read)) CRITFE(close);
    return status;
trap_pipe:
trap_nmax:
trap_pollfd_buffer_null:
trap_fd_info_buffer_null:
trap_fi_null:
    return thrd_error;
}

int fdset_input_close(struct fdset_input *fi, int_fast8_t timeout_detach)
{
    TRAPNULL(fi);
    fi->close = -1;
    int status = close(fi->pipefd_write) ? thrd_error : thrd_success;
    if(status!=thrd_success) CRITFE(close);
    int thread_res;
    int status_join = lgk_thread_join(&fi->thread, &thread_res, fi->timeout_ms, timeout_detach);
    if(status_join!=thrd_success)
    {
        CRITF(lgk_thread_join, "%i", status_join);
        status = status_join;
    }
    else if(thread_res)
    {
        CRITF(fdset_input_thread, "%i", thread_res);
        status = thrd_error;
    }
    int status_thread_mon_destroy = lgk_monitor_destroy(&fi->thread_mon);
    if(status_thread_mon_destroy)
    {
        CRITF(lgk_monitor_destroy, "%i", status_thread_mon_destroy);
        if(status==thrd_success) status = status_thread_mon_destroy;
    }
    mtx_destroy(&fi->mutex);
    return status;
trap_fi_null:
    return thrd_error;
}

int fdset_input_async_add_fd(struct fdset_input *fi, int fd, void *buffer, unsigned bufsize, fdset_input_callback *cb, void *arg)
{
    TRAP(fi->nused >= fi->nmax, full, "nmax==%u, nused==%u", fi->nmax, fi->nused);
    ssize_t nwritten = fd_write_timed(fi->pipefd_write, &(uint8_t){THREAD_CMD_PAUSE}, 1, fi->timeout_ms, NULL);
    TRAPFS(!nwritten, fd_write_timed, timeout_pause, "%s", "timeout");
    TRAPFES(nwritten<0, fd_write_timed, pause);
    int status = mtx_timedlock_ms(&fi->mutex, fi->timeout_ms);
    TRAPF(status!=thrd_success, mtx_timedlock_ms, "%i", status);
    int index = fi->nused++;
    fi->fd_info_buffer[index] = (struct fdset_input_fd_info){cb, arg, bufsize, 0, buffer};
    fi->pollfd_buffer[index+1] = (struct pollfd){fd, POLLIN, 0};
    status = mtx_unlock(&fi->mutex);
    TRAPF(status!=thrd_success, mtx_unlock, "%i", status);
    nwritten = fd_write_timed(fi->pipefd_write, &(uint8_t){THREAD_CMD_RUN}, 1, fi->timeout_ms, NULL);
    TRAPFS(!nwritten, fd_write_timed, timeout_run, "%s", "timeout");
    TRAPFES(nwritten<0, fd_write_timed, run);
    return 0;
trap_fd_write_timed_run:
trap_fd_write_timed_timeout_run:
trap_mtx_unlock:
trap_mtx_timedlock_ms:
trap_fd_write_timed_pause:
trap_fd_write_timed_timeout_pause:
trap_full:
    return -1;
}

int fdset_input_async_remove_fd(struct fdset_input *fi, int fd)
{
    TRAP(!fi->nused, empty, "nused==%u", fi->nused);
    unsigned index;
    for(index=0; index<fi->nmax; index++) if(fi->pollfd_buffer[index+1].fd == fd) break;
    TRAP(index>=fi->nmax, invalid_fd, "invalid fd: %i", fd);
    ssize_t nwritten = fd_write_timed(fi->pipefd_write, &(uint8_t){THREAD_CMD_PAUSE}, 1, fi->timeout_ms, NULL);
    TRAPFS(!nwritten, fd_write_timed, timeout_pause, "%s", "timeout");
    TRAPFES(nwritten < 0, fd_write_timed, pause);
    int status = mtx_timedlock_ms(&fi->mutex, fi->timeout_ms);
    TRAPF(status!=thrd_success, mtx_timedlock_ms, "%i", status);
    fi->pollfd_buffer[index+1] = fi->pollfd_buffer[fi->nused--];
    fi->fd_info_buffer[index] = fi->fd_info_buffer[fi->nused];
    status = mtx_unlock(&fi->mutex);
    TRAPF(status!=thrd_success, mtx_unlock, "%i", status);
    nwritten = fd_write_timed(fi->pipefd_write, &(uint8_t){THREAD_CMD_RUN}, 1, fi->timeout_ms, NULL);
    TRAPFS(!nwritten, fd_write_timed, timeout_run, "%s", "timeout");
    TRAPFES(nwritten<0, fd_write_timed, run);
    return 0;
trap_fd_write_timed_run:
trap_fd_write_timed_timeout_run:
trap_mtx_unlock:
trap_mtx_timedlock_ms:
trap_fd_write_timed_pause:
trap_fd_write_timed_timeout_pause:
trap_invalid_fd:
trap_empty:
    return -1;
}
