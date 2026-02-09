#ifndef FDSET_INPUT_H
#define FDSET_INPUT_H

#include <threads.h>
#include <poll.h>

#define FDSET_INPUT_STATIC(name, nmax)\
    struct fdset_input name;\
    struct fdset_input_fd_info name##_fd_info_buffer[nmax];\
    struct pollfd name##_pollfd_buffer[nmax+1]

enum fdset_input_callback_action : uint_fast8_t
{
    FDSET_INPUT_CALLBACK_ACTION_NONE = 0,
    FDSET_INPUT_CALLBACK_ACTION_CLEAR_BUFFER,
    FDSET_INPUT_CALLBACK_ACTION_REMOVE_FD,

    N_FDSET_INPUT_CALLBACK_ACTIONS
};

typedef enum fdset_input_callback_action fdset_input_callback(int fd, void *buf, unsigned nbuf, int_fast8_t err);

struct fdset_input_fd_info
{
    fdset_input_callback *callback;
    unsigned buffer_size;
    unsigned buffer_used;
    void *buffer;
};

struct fdset_input
{
    struct fdset_input_fd_info *fd_info_buffer;
    struct pollfd *pollfd_buffer;
    unsigned nmax;
    unsigned nused;
    unsigned poll_timeout_ms;
    unsigned lock_timeout_ms;
    union
    {
        int pipefd[2];
        struct
        {
            int pipefd_read;
            int pipefd_write;
        };
    };
    mtx_t mutex;
    thrd_t thread;
};

int fdset_input_init(struct fdset_input *fi, struct fdset_input_fd_info *fd_info_buffer, struct pollfd *pollfd_buffer, unsigned nmax, unsigned poll_timeout_ms, unsigned lock_timeout_ms);
int fdset_input_close(struct fdset_input *fi, int *thread_res);
int fdset_input_async_add_fd(struct fdset_input *fi, int fd, void *buffer, unsigned bufsize, fdset_input_callback *cb);
int fdset_input_async_remove_fd(struct fdset_input *fi, int fd);

#endif
