#include <errno.h>
#include <unistd.h>

#include "utils.h"

ssize_t xread(int fd, void *buf, size_t count)
{
  // Read some bytes. Retry on EINTR and when we don't get as many bytes as we requested.
  size_t alreadyRead = 0;
  start:;
  ssize_t res = read(fd, buf, count);
  if (res == -1 && errno == EINTR) goto start;
  if (res > 0) {
    buf = ((char*)buf)+res;
    count -= res;
    alreadyRead += res;
  }
  if (res == -1) return -1;
  if (count == 0 || res == 0) return alreadyRead;
  goto start;
}
