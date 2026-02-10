#ifndef LGK_FD_H
#define LGK_FD_H

/* readloop and writeloop are currently broken (loop bug and error handling) and need to be fixed or rewritten. */
ssize_t readloop(int fd, void *buf, size_t count);
ssize_t writeloop(int fd, void *buf, size_t count);

#endif
