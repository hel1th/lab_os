#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define KSIZE 3
#define K_ITER 10

float **A;
float **B;
float kernel[KSIZE][KSIZE] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};

int rows, cols;
int threadCount;

void *apply_kernel_thread(void *arg) {
  int id = *(int *)arg;
  int chunk = rows / threadCount;
  int start = id * chunk;
  int end = (id == threadCount - 1) ? rows : start + chunk;
  int offset = KSIZE / 2;

  for (int i = start; i < end; ++i) {
    for (int j = 0; j < cols; ++j) {
      float sum = 0.0f;
      for (int u = 0; u < KSIZE; ++u) {
        for (int v = 0; v < KSIZE; ++v) {
          int x = i + u - offset;
          int y = j + v - offset;
          float val = 0.0f;
          if (x >= 0 && x < rows && y >= 0 && y < cols)
            val = A[x][y];
          sum += val * kernel[u][v];
        }
      }
      B[i][j] = sum;
    }
  }
  return NULL;
}

float **alloc_matrix(int r, int c) {
  float **m = malloc(r * sizeof(float *));
  for (int i = 0; i < r; ++i)
    m[i] = malloc(c * sizeof(float));
  return m;
}

void free_matrix(float **m, int r) {
  for (int i = 0; i < r; ++i)
    free(m[i]);
  free(m);
}

void print_matrix(float **m, int r, int c) {
  for (int i = 0; i < r; ++i) {
    for (int j = 0; j < c; ++j)
      printf("%6.2f ", m[i][j]);
    printf("\n");
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc > 4) {
    printf("Usage: %s <num_threads> <rows> <cols>\n", argv[0]);
    return 1;
  } else if (argc < 3) {
    printf("Usage: %s <num_threads> <rows> (for square matrix)\n", argv[0]);
    return 1;
  }

  rows = atoi(argv[2]);
  cols = argc == 4 ? atoi(argv[3]) : rows;

  if (rows <= 0 || cols <= 0) {
    printf("Invalid matrix size.\n");
    return 1;
  }

  threadCount = atoi(argv[1]);
  if (threadCount < 1 || threadCount > rows) {
    printf("Invalid thread count. Must be between 1 and %d.\n", rows);
    return 1;
  }

  A = alloc_matrix(rows, cols);
  B = alloc_matrix(rows, cols);

  srand(time(NULL));
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < cols; ++j)
      A[i][j] = (rand() % 100000) / 1000.0f;

  // printf("Initial matrix:\n");
  // print_matrix(A, rows, cols);

  pthread_t threads[threadCount];
  int ids[threadCount];

  struct timeval start, end;
  gettimeofday(&start, NULL);

  for (int k = 0; k < K_ITER; ++k) {
    for (int t = 0; t < threadCount; ++t) {
      ids[t] = t;
      pthread_create(&threads[t], NULL, apply_kernel_thread, &ids[t]);
    }

    for (int t = 0; t < threadCount; ++t)
      pthread_join(threads[t], NULL);

    for (int i = 0; i < rows; ++i)
      for (int j = 0; j < cols; ++j)
        A[i][j] = B[i][j];
  }

  gettimeofday(&end, NULL);
  double durationOfComputing = (end.tv_sec - start.tv_sec) * 1000.0 +
                               (end.tv_usec - start.tv_usec) / 1000.0;

  printf("Result after %d iterations:\n", K_ITER);
  // print_matrix(A, rows, cols);

  printf("Computing matrix %dx%d time with %d threads is %.3lf ms\n", rows,
         cols, threadCount, durationOfComputing);

  free_matrix(A, rows);
  free_matrix(B, rows);

  return 0;
}
