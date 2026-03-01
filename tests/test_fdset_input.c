/*
 * Tests for fdset_input: pipes, add fd, write data, verify it is read and
 * delivered via callback. Run: ctest (from build dir).
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#include <lgk_fdset_input.h>

#define NMAX 32
#define BUF_SIZE 256
#define TEST_MSG "hello fdset_input"
#define FDSET_TIMEOUT_MS 15000
#define WAIT_FOR_CALLBACK_SEC 5

static char received_buf[BUF_SIZE];
static unsigned received_len;
static mtx_t received_mtx;
static cnd_t received_cnd;
static int received_done;

static enum fdset_input_callback_action on_data(int fd, void *buf, unsigned nbuf, enum fdset_input_status status, void *arg)
{
    (void)fd;
    (void)arg;
    assert(status == FDSET_INPUT_STATUS_OK);
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

    status = fdset_input_init(&fi, fd_info_buffer, pollfd_buffer, NMAX, FDSET_TIMEOUT_MS);
    assert(status == thrd_success);

    status = fdset_input_async_add_fd(&fi, pipefd[0], pipe_buf, BUF_SIZE, on_data, NULL);
    assert(status == 0);

    /* Write a larger payload: repeat message many times to move more data. */
    {
        const char *msg = TEST_MSG;
        size_t one = strlen(msg) + 1;
        size_t len = 0;
        for (int i = 0; i < 50; i++) {
            ssize_t n = write(pipefd[1], msg, one);
            assert((size_t)n == one);
            len += one;
        }
        (void)len;
    }

    /* Wait for callback (with timeout). */
    mtx_lock(&received_mtx);
    while (!received_done) {
        struct timespec ts;
        timespec_get(&ts, TIME_UTC);
        ts.tv_sec += WAIT_FOR_CALLBACK_SEC;
        int r = cnd_timedwait(&received_cnd, &received_mtx, &ts);
        assert(r == thrd_success || r == thrd_timedout);
        if (r == thrd_timedout)
            break;
    }
    mtx_unlock(&received_mtx);

    assert(received_done);
    /* We may get one or more callbacks; buffer should end with the message (last write). */
    assert(received_len >= strlen(TEST_MSG) + 1);
    assert(memcmp(received_buf, TEST_MSG, strlen(TEST_MSG) + 1) == 0);

    /* Close fdset first so the thread stops; then close pipe write end (avoid POLLHUP while polling).
     * Give the worker a moment to see POLLHUP before we block in join. */
    thrd_sleep(&(struct timespec){ .tv_sec = 0, .tv_nsec = 50 * 1000 * 1000 }, NULL);
    status = fdset_input_close(&fi, 0);
    assert(status == thrd_success);
    close(pipefd[1]);

    cnd_destroy(&received_cnd);
    mtx_destroy(&received_mtx);
}

static char received_a[BUF_SIZE], received_b[BUF_SIZE];
static unsigned len_a, len_b;
static int done_a, done_b;

static enum fdset_input_callback_action on_data_a(int fd, void *buf, unsigned nbuf, enum fdset_input_status status, void *arg)
{
    (void)fd;
    (void)arg;
    assert(status == FDSET_INPUT_STATUS_OK);
    memcpy(received_a, buf, nbuf);
    len_a = nbuf;
    done_a = 1;
    return FDSET_INPUT_CALLBACK_ACTION_NONE;
}

static enum fdset_input_callback_action on_data_b(int fd, void *buf, unsigned nbuf, enum fdset_input_status status, void *arg)
{
    (void)fd;
    (void)arg;
    assert(status == FDSET_INPUT_STATUS_OK);
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

    status = fdset_input_init(&fi, fd_info_buffer, pollfd_buffer, NMAX, FDSET_TIMEOUT_MS);
    assert(status == thrd_success);

    status = fdset_input_async_add_fd(&fi, pipefd_a[0], pipe_buf_a, BUF_SIZE, on_data_a, NULL);
    assert(status == 0);
    status = fdset_input_async_add_fd(&fi, pipefd_b[0], pipe_buf_b, BUF_SIZE, on_data_b, NULL);
    assert(status == 0);

    /* More data per pipe: send many chunks. */
    for (int i = 0; i < 100; i++) {
        assert(write(pipefd_a[1], "A", 2) == 2);
        assert(write(pipefd_b[1], "B", 2) == 2);
    }

    /* Wait for both callbacks (under load callbacks can be delayed). */
    for (int i = 0; i < (WAIT_FOR_CALLBACK_SEC * 10); i++) {
        thrd_sleep(&(struct timespec){ .tv_sec = 0, .tv_nsec = 100 * 1000 * 1000 }, NULL);
        if (done_a && done_b)
            break;
    }

    assert(done_a);
    assert(done_b);
    /* Each pipe got 100 * 2 bytes; we only check we got data and it looks right. */
    assert(len_a >= 2 && received_a[0] == 'A');
    assert(len_b >= 2 && received_b[0] == 'B');

    thrd_sleep(&(struct timespec){ .tv_sec = 0, .tv_nsec = 50 * 1000 * 1000 }, NULL);
    status = fdset_input_close(&fi, 0);
    assert(status == thrd_success);
    close(pipefd_a[1]);
    close(pipefd_b[1]);
}

/* All NMAX pipes active, each receiving a lot of data. Accumulate per pipe to verify content.
 * Per-pipe read buffer must fit at least one full message stream so we get "i-99" in one callback. */
#define N_PIPES NMAX
#define N_ROUNDS 100
#define PIPE_READ_BUF_SIZE 1024   /* > N_ROUNDS * 6 so one read can get full stream per pipe */
static char recv_bufs[N_PIPES][PIPE_READ_BUF_SIZE];
static unsigned recv_lens[N_PIPES];
static int recv_done[N_PIPES];
static mtx_t recv_mtx;
static cnd_t recv_cnd;

static enum fdset_input_callback_action on_data_idx(int fd, void *buf, unsigned nbuf, enum fdset_input_status status, void *arg)
{
    (void)fd;
    int idx = (int)(intptr_t)arg;
    assert(status == FDSET_INPUT_STATUS_OK);
    assert(idx >= 0 && idx < N_PIPES);
    assert(nbuf <= PIPE_READ_BUF_SIZE);
    mtx_lock(&recv_mtx);
    /* Overwrite: we only need the latest chunk (fdset_input gives us full buffer each time). */
    recv_lens[idx] = nbuf;
    memcpy(recv_bufs[idx], buf, nbuf);
    recv_done[idx] = 1;
    cnd_broadcast(&recv_cnd);
    mtx_unlock(&recv_mtx);
    return FDSET_INPUT_CALLBACK_ACTION_NONE;
}

static void test_four_pipes_more_data(void)
{
    struct fdset_input fi;
    struct fdset_input_fd_info fd_info_buffer[NMAX];
    struct pollfd pollfd_buffer[NMAX + 1];
    char pipe_bufs[N_PIPES][PIPE_READ_BUF_SIZE];
    int pipefds[N_PIPES][2];
    int status;

    memset(recv_done, 0, sizeof recv_done);
    memset(recv_lens, 0, sizeof recv_lens);
    memset(recv_bufs, 0, sizeof recv_bufs);
    assert(mtx_init(&recv_mtx, mtx_plain) == thrd_success);
    assert(cnd_init(&recv_cnd) == thrd_success);

    for (int i = 0; i < N_PIPES; i++)
        assert(pipe(pipefds[i]) == 0);

    status = fdset_input_init(&fi, fd_info_buffer, pollfd_buffer, NMAX, FDSET_TIMEOUT_MS);
    assert(status == thrd_success);

    for (int i = 0; i < N_PIPES; i++) {
        status = fdset_input_async_add_fd(&fi, pipefds[i][0], pipe_bufs[i], PIPE_READ_BUF_SIZE,
                                          on_data_idx, (void *)(intptr_t)i);
        assert(status == 0);
    }

    /* All pipes: many writes each; last message to pipe i is "i-(N_ROUNDS-1)\0". */
    for (int r = 0; r < N_ROUNDS; r++) {
        for (int i = 0; i < N_PIPES; i++) {
            char msg[16];
            int n = (int)snprintf(msg, sizeof msg, "%d-%d", i, r);
            assert(n > 0 && (size_t)n < sizeof msg);
            ssize_t w = write(pipefds[i][1], msg, (size_t)n + 1);
            assert((size_t)w == (size_t)n + 1);
        }
    }

    /* Wait until every pipe has received the final message "i-(N_ROUNDS-1)". */
    {
        char expected_buf[N_PIPES][24];
        size_t expected_len[N_PIPES];
        for (int i = 0; i < N_PIPES; i++) {
            int n = (int)snprintf(expected_buf[i], sizeof expected_buf[i], "%d-%d", i, N_ROUNDS - 1);
            assert(n > 0);
            expected_len[i] = (size_t)n;
        }
        mtx_lock(&recv_mtx);
        for (int deadline = 0; deadline < WAIT_FOR_CALLBACK_SEC * 20; deadline++) {
            int all = 1;
            for (int i = 0; i < N_PIPES; i++) {
                int found = 0;
                for (size_t j = 0; j + expected_len[i] <= recv_lens[i]; j++)
                    if (memcmp(recv_bufs[i] + j, expected_buf[i], expected_len[i]) == 0) { found = 1; break; }
                if (!found) { all = 0; break; }
            }
            if (all) break;
            struct timespec ts;
            timespec_get(&ts, TIME_UTC);
            ts.tv_sec += 1;
            cnd_timedwait(&recv_cnd, &recv_mtx, &ts);
        }
        mtx_unlock(&recv_mtx);
    }

    /* Verify received data: we got the final message "i-(N_ROUNDS-1)" for each pipe. */
    for (int i = 0; i < N_PIPES; i++) {
        char expected[24];
        int n = (int)snprintf(expected, sizeof expected, "%d-%d", i, N_ROUNDS - 1);
        assert(n > 0);
        size_t len = (size_t)n;
        int found = 0;
        for (size_t j = 0; j + len <= recv_lens[i]; j++)
            if (memcmp(recv_bufs[i] + j, expected, len) == 0) { found = 1; break; }
        assert(found && "received data does not contain last message written to this pipe");
    }

    thrd_sleep(&(struct timespec){ .tv_sec = 0, .tv_nsec = 50 * 1000 * 1000 }, NULL);
    status = fdset_input_close(&fi, 0);
    assert(status == thrd_success);
    for (int i = 0; i < N_PIPES; i++)
        close(pipefds[i][1]);

    cnd_destroy(&recv_cnd);
    mtx_destroy(&recv_mtx);
}

int main(void)
{
    test_pipe_data_gets_through();
    test_two_pipes_both_get_data();
    test_four_pipes_more_data();
    return 0;
}
