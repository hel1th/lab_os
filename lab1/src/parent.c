#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define POOL 4096

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <child1> <child2>\n", argv[0]);
    return 3;
  }
  const char *child1Path = argv[1];
  const char *child2Path = argv[2];

  // 0 - read end | 1 - write end
  int pipe1[2], pipe2[2], pipe12[2];
  pipe(pipe1);  // parent to c1
  pipe(pipe2);  // c2 to parent
  pipe(pipe12); // c1 to c2

  if (pipe(pipe1) == -1 || pipe(pipe12) == -1 || pipe(pipe2) == -1) {
    printf("Error occured during opening pipes!\n");
    return 1;
  }

  const pid_t pid1 = fork();
  if (pid1 == 0) {
    // dup2 from fd2 in child to fd  STDIN_FILENO -> pipe12
    dup2(pipe1[0], STDIN_FILENO);
    dup2(pipe12[1], STDOUT_FILENO);

    close(pipe1[1]);
    close(pipe12[0]);
    close(pipe2[0]);
    close(pipe2[1]);

    execl(child1Path, child1Path, NULL);
    printf("Something went wrong during executing child1");
    return 2;
  }

  const pid_t pid2 = fork();
  if (pid2 == 0) {
    dup2(pipe12[0], STDIN_FILENO);
    dup2(pipe2[1], STDOUT_FILENO);

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe12[1]);
    close(pipe2[0]);

    execl(child2Path, child2Path, NULL);
    printf("Something went wrong during executing child2");
    return 2;
  }
  close(pipe1[0]);
  close(pipe12[0]);
  close(pipe12[1]);
  close(pipe2[1]);

  char buff[POOL];
  ssize_t nBytes;

  while ((nBytes = read(STDERR_FILENO, buff, sizeof(buff)))) {
    if (nBytes < 0) {
      const char msg[] = "error: failed to read from stdin\n";
      write(STDERR_FILENO, msg, sizeof(msg));
      return 4;
    } else if (buff[0] == '\n') {
      break;
    }

    write(pipe1[1], buff, nBytes);

    nBytes = read(pipe2[0], buff, sizeof(buff));
    write(STDERR_FILENO, buff, nBytes);
  }

  close(pipe1[1]);
  close(pipe2[0]);

  wait(NULL);
  wait(NULL);
}
