#include <ctype.h>
#include <unistd.h>

int main() {
  char buf[1024];
  ssize_t n;
  while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
    for (ssize_t i = 0; i < n; i++)
      if (isspace(buf[i]) && buf[i] != '\n')
        buf[i] = '_';
    write(STDOUT_FILENO, buf, n);
  }
}
