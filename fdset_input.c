#include <stdint.h>
#include <threads.h>
#include <unistd.h>
#include <lgk_tnt.h>
#include <lgk_threads.h>
#include <lgk_fdset_input.h>

#define FDSET_INPUT_DEFAULT_TIMEOUT_MS 1000

constexpr uint8_t THREAD_CMD_RUN = 0;
constexpr uint8_t THREAD_CMD_PAUSE = 1;

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

static void callback_exec(struct fdset_input *fi, unsigned fd_index, int_fast8_t err)
{
    static callback_action *const callback_action_table[N_FDSET_INPUT_CALLBACK_ACTIONS] =
    {
        NULL,
        callback_action_clear_buffer,
        callback_action_remove_fd
    };
    struct fdset_input_fd_info *fd_info = fi->fd_info_buffer + fd_index;
    struct pollfd *pollfd = fi->pollfd_buffer + fd_index + 1;
    enum fdset_input_callback_action action = fd_info->callback(pollfd->fd, fd_info->buffer, fd_info->buffer_used, err);
    TRAP(action>=N_FDSET_INPUT_CALLBACK_ACTIONS, action_invalid, "action==%i", action);
    if(callback_action_table[action]) callback_action_table[action](fi, fd_index);
trap_action_invalid:
}

static int fdset_input_thread(void *data)
{
    uint8_t cmd = THREAD_CMD_RUN;
    constexpr short ERRMASK = POLLERR | POLLNVAL;
    struct fdset_input *restrict fi = data;
    int_fast8_t exit = 0;
    while(!exit)
    {
        while(cmd == THREAD_CMD_PAUSE)
        {
            read(fi->pipefd_read, &cmd, 1);
        }
        int err = mtx_timedlock_ms(&fi->mutex, fi->lock_timeout_ms);
        TRAPF(err!=thrd_success, mtx_timedlock_ms, "%i", err);
        int nevents = poll(fi->pollfd_buffer, fi->nused+1, (int)fi->poll_timeout_ms);
        TRAPFE(nevents<0, poll);
        short pipe_revents = fi->pollfd_buffer[0].revents;
        if(pipe_revents)
        {
            if(pipe_revents & POLLHUP) exit = -1;
            if(pipe_revents | POLLIN) read(fi->pipefd_read, &cmd, 1);
            if(pipe_revents & ERRMASK) ERR("errors reported in pipe_revents: %04hx", pipe_revents);
            if(pipe_revents & ~(ERRMASK | POLLIN)) ERR("unhandled flags in pipe_revents: %04hx", pipe_revents);
            nevents--;
        }
        for(unsigned i=0; nevents && (i<fi->nused); i++)
        {
            struct pollfd *pollfd = fi->pollfd_buffer+i+1;
            short revents = pollfd->revents;
            if(revents)
            {
                struct fdset_input_fd_info *fd_info = fi->fd_info_buffer + i;
                if(revents & POLLIN)
                {
                    uint8_t *buf_read = ((uint8_t*)fd_info->buffer) + fd_info->buffer_used;
                    ssize_t nread = read(pollfd->fd, buf_read, fd_info->buffer_size - fd_info->buffer_used);
                    if(nread>0) fd_info->buffer_used += nread;
                    if(fd_info->callback) callback_exec(fi, i, 0);
                }
                if(revents & ERRMASK) if(fd_info->callback) callback_exec(fi, i, -1);
                if(revents & ~(ERRMASK | POLLIN)) ERR("unhandled flags in revents: %04hx", revents);
                nevents--;
            }
        }
        err = mtx_unlock(&fi->mutex);
        TRAPF(err!=thrd_success, mtx_unlock, "%i", err);
    }
    close(fi->pipefd_read);
    return 0;
trap_mtx_unlock:
trap_poll:
trap_mtx_timedlock_ms:
    close(fi->pipefd_read);
    return -1;
}

// TODO: DOC pollfd_buffer size has to be nmax+1 (one element for pipe)
int fdset_input_init(struct fdset_input *const fi, struct fdset_input_fd_info *const fd_info_buffer, struct pollfd *const pollfd_buffer, unsigned nmax, unsigned poll_timeout_ms, unsigned lock_timeout_ms)
{
    TRAP(!nmax, nmax, "nmax==%i", nmax);
    fi->fd_info_buffer = fd_info_buffer;
    fi->pollfd_buffer = pollfd_buffer;
    fi->nmax = nmax;
    fi->nused = 0;
    fi->poll_timeout_ms = (poll_timeout_ms == 0) ? FDSET_INPUT_DEFAULT_TIMEOUT_MS : poll_timeout_ms;
    fi->lock_timeout_ms = (lock_timeout_ms == 0) ? FDSET_INPUT_DEFAULT_TIMEOUT_MS : lock_timeout_ms;
    int err = pipe(fi->pipefd);
    TRAPFE(err, pipe);
    pollfd_buffer[0] = (struct pollfd){fi->pipefd_read, POLLIN, 0};
    err = mtx_init(&fi->mutex, mtx_timed);
    TRAPF(err!=thrd_success, mtx_init, "%i", err);
    err = thrd_create(&fi->thread, fdset_input_thread, fi);
    TRAPF(err!=thrd_success, thrd_create, "%i", err);
    return 0;
trap_thrd_create:
    mtx_destroy(&fi->mutex);
trap_mtx_init:
    close(fi->pipefd_write);
    close(fi->pipefd_read);
trap_pipe:
trap_nmax:
    return -1;
}

int fdset_input_close(struct fdset_input *fi, int *thread_res)
{
    close(fi->pipefd_write);
    int err = thrd_join(fi->thread, thread_res);
    TRAPF(err!=thrd_success, thrd_join, "%i", err);
    mtx_destroy(&fi->mutex);
    return 0;
trap_thrd_join:
    return -1;
}

int fdset_input_async_add_fd(struct fdset_input *fi, int fd, void *buffer, unsigned bufsize, fdset_input_callback *cb)
{
    TRAP(fi->nused >= fi->nmax, full, "nmax==%i, nused==%i", fi->nmax, fi->nused);
    uint8_t cmd = THREAD_CMD_PAUSE;
    write(fi->pipefd_write, &cmd, 1);
    int err = mtx_timedlock_ms(&fi->mutex, fi->lock_timeout_ms);
    TRAPF(err!=thrd_success, mtx_timedlock_ms, "%i", err);
    int index = fi->nused++;
    fi->fd_info_buffer[index] = (struct fdset_input_fd_info){cb, bufsize, 0, buffer};
    fi->pollfd_buffer[index+1] = (struct pollfd){fd, POLLIN, 0};
    err = mtx_unlock(&fi->mutex);
    TRAPF(err!=thrd_success, mtx_unlock, "%i", err);
    write(fi->pipefd_write, &(uint8_t){THREAD_CMD_RUN}, 1);
    return 0;
trap_mtx_unlock:
trap_mtx_timedlock_ms:
trap_full:
    return -1;
}

int fdset_input_async_remove_fd(struct fdset_input *fi, int fd)
{
    // TODO TRAP(fi->nmax >= fi->nused, full, "nmax==%i, nused==%i", c->nmax, c->nused);
    unsigned index;
    for(index=0; index<fi->nmax; index++) if(fi->pollfd_buffer[index+1].fd == fd) break;
    TRAP(index>=fi->nmax, invalid_fd, "invalid fd: %i", fd);
    uint8_t cmd = THREAD_CMD_PAUSE;
    write(fi->pipefd_write, &cmd, 1);
    int err = mtx_timedlock_ms(&fi->mutex, fi->lock_timeout_ms);
    TRAPF(err!=thrd_success, mtx_timedlock_ms, "%i", err);
    fi->pollfd_buffer[index+1] = fi->pollfd_buffer[fi->nused--];
    fi->fd_info_buffer[index] = fi->fd_info_buffer[fi->nused];
    err = mtx_unlock(&fi->mutex);
    TRAPF(err!=thrd_success, mtx_unlock, "%i", err);
    write(fi->pipefd_write, &(uint8_t){THREAD_CMD_RUN}, 1);
    return 0;
trap_mtx_unlock:
trap_mtx_timedlock_ms:
trap_invalid_fd:
    return -1;
}
