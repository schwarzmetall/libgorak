#ifndef FDSET_INPUT_H
#define FDSET_INPUT_H

#include <stdint.h>
#include <threads.h>
#include <poll.h>
#include <lgk_threads.h>

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

enum fdset_input_error : uint_fast8_t
{
    FDSET_INPUT_ERROR_NONE = 0,
    FDSET_INPUT_ERROR_POLL,
    FDSET_INPUT_ERROR_READ,

    N_FDSET_INPUT_ERRORS
};

typedef enum fdset_input_callback_action fdset_input_callback(int fd, void *buf, unsigned nbuf, enum fdset_input_error err);

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
    int timeout_ms;
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
    struct lgk_thread thread;
    struct lgk_monitor thread_mon;
    int_fast8_t close;
};

/* pollfd_buffer must have length at least nmax + 1 (one element for the internal pipe). */
int fdset_input_init(struct fdset_input *fi, struct fdset_input_fd_info *fd_info_buffer, struct pollfd *pollfd_buffer, unsigned nmax, int timeout_ms);
int fdset_input_close(struct fdset_input *fi, int timeout_ms, int_fast8_t timeout_detach);
int fdset_input_async_add_fd(struct fdset_input *fi, int fd, void *buffer, unsigned bufsize, fdset_input_callback *cb);
int fdset_input_async_remove_fd(struct fdset_input *fi, int fd);

#endif
