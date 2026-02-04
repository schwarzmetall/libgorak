#ifndef LGK_FD_H
#define LGK_FD_H

ssize_t readloop(int fd, void *buf, size_t count);
ssize_t writeloop(int fd, void *buf, size_t count);

#endif
