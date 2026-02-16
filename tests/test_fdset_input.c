/*
 * Tests for fdset_input: pipes, add fd, write data, verify it is read and
 * delivered via callback. Run: ctest (from build dir).
 */

#include <assert.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#include <lgk_fdset_input.h>

#define NMAX 4
#define BUF_SIZE 256
#define TEST_MSG "hello fdset_input"

static char received_buf[BUF_SIZE];
static unsigned received_len;
static mtx_t received_mtx;
static cnd_t received_cnd;
static int received_done;

static enum fdset_input_callback_action on_data(int fd, void *buf, unsigned nbuf, enum fdset_input_error err)
{
    (void)fd;
    assert(err == FDSET_INPUT_ERROR_NONE);
    assert(nbuf <= BUF_SIZE);
    mtx_lock(&received_mtx);
    received_len = nbuf;
    memcpy(received_buf, buf, nbuf);
    received_done = 1;
    cnd_signal(&received_cnd);
    mtx_unlock(&received_mtx);
    return FDSET_INPUT_CALLBACK_ACTION_NONE;
}

static void test_pipe_data_gets_through(void)
{
    struct fdset_input fi;
    struct fdset_input_fd_info fd_info_buffer[NMAX];
    struct pollfd pollfd_buffer[NMAX + 1];
    char pipe_buf[BUF_SIZE];
    int pipefd[2];
    int status;

    received_len = 0;
    received_done = 0;
    memset(received_buf, 0, sizeof received_buf);
    assert(mtx_init(&received_mtx, mtx_plain) == thrd_success);
    assert(cnd_init(&received_cnd) == thrd_success);

    assert(pipe(pipefd) == 0);

    status = fdset_input_init(&fi, fd_info_buffer, pollfd_buffer, NMAX, 2000);
    assert(status == thrd_success);

    status = fdset_input_async_add_fd(&fi, pipefd[0], pipe_buf, BUF_SIZE, on_data);
    assert(status == 0);

    /* Write test message to the pipe; fdset_input thread should read it and call on_data. */
    {
        const char *msg = TEST_MSG;
        size_t len = strlen(msg) + 1;
        ssize_t n = write(pipefd[1], msg, len);
        assert((size_t)n == len);
    }

    /* Wait for callback (with timeout). */
    mtx_lock(&received_mtx);
    while (!received_done) {
        struct timespec ts;
        timespec_get(&ts, TIME_UTC);
        ts.tv_sec += 2;
        int r = cnd_timedwait(&received_cnd, &received_mtx, &ts);
        assert(r == thrd_success || r == thrd_timedout);
        if (r == thrd_timedout)
            break;
    }
    mtx_unlock(&received_mtx);

    assert(received_done);
    assert(received_len == strlen(TEST_MSG) + 1);
    assert(memcmp(received_buf, TEST_MSG, received_len) == 0);

    /* Close fdset first so the thread stops; then close pipe write end (avoid POLLHUP while polling). */
    status = fdset_input_close(&fi, 0);
    assert(status == thrd_success);
    close(pipefd[1]);

    cnd_destroy(&received_cnd);
    mtx_destroy(&received_mtx);
}

static char received_a[BUF_SIZE], received_b[BUF_SIZE];
static unsigned len_a, len_b;
static int done_a, done_b;

static enum fdset_input_callback_action on_data_a(int fd, void *buf, unsigned nbuf, enum fdset_input_error err)
{
    (void)fd;
    assert(err == FDSET_INPUT_ERROR_NONE);
    memcpy(received_a, buf, nbuf);
    len_a = nbuf;
    done_a = 1;
    return FDSET_INPUT_CALLBACK_ACTION_NONE;
}

static enum fdset_input_callback_action on_data_b(int fd, void *buf, unsigned nbuf, enum fdset_input_error err)
{
    (void)fd;
    assert(err == FDSET_INPUT_ERROR_NONE);
    memcpy(received_b, buf, nbuf);
    len_b = nbuf;
    done_b = 1;
    return FDSET_INPUT_CALLBACK_ACTION_NONE;
}

static void test_two_pipes_both_get_data(void)
{
    struct fdset_input fi;
    struct fdset_input_fd_info fd_info_buffer[NMAX];
    struct pollfd pollfd_buffer[NMAX + 1];
    char pipe_buf_a[BUF_SIZE];
    char pipe_buf_b[BUF_SIZE];
    int pipefd_a[2], pipefd_b[2];
    int status;

    len_a = len_b = 0;
    done_a = done_b = 0;
    memset(received_a, 0, sizeof received_a);
    memset(received_b, 0, sizeof received_b);

    assert(pipe(pipefd_a) == 0);
    assert(pipe(pipefd_b) == 0);

    status = fdset_input_init(&fi, fd_info_buffer, pollfd_buffer, NMAX, 2000);
    assert(status == thrd_success);

    status = fdset_input_async_add_fd(&fi, pipefd_a[0], pipe_buf_a, BUF_SIZE, on_data_a);
    assert(status == 0);
    status = fdset_input_async_add_fd(&fi, pipefd_b[0], pipe_buf_b, BUF_SIZE, on_data_b);
    assert(status == 0);

    assert(write(pipefd_a[1], "A", 2) == 2);
    assert(write(pipefd_b[1], "B", 2) == 2);

    /* Brief wait for both callbacks. */
    for (int i = 0; i < 50; i++) {
        thrd_sleep(&(struct timespec){ .tv_sec = 0, .tv_nsec = 100 * 1000 * 1000 }, NULL);
        if (done_a && done_b)
            break;
    }

    assert(done_a);
    assert(done_b);
    assert(len_a == 2 && received_a[0] == 'A' && received_a[1] == '\0');
    assert(len_b == 2 && received_b[0] == 'B' && received_b[1] == '\0');

    status = fdset_input_close(&fi, 0);
    assert(status == thrd_success);
    close(pipefd_a[1]);
    close(pipefd_b[1]);
}

int main(void)
{
    test_pipe_data_gets_through();
    test_two_pipes_both_get_data();
    return 0;
}
