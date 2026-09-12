#include <stdio.h>
#define MAX 100

int add(int a, int b) {
  return a + b;
}

int main(void) {
  int total = 0;
  char *msg = "hello";
  // sum the first ten numbers
  for (int i = 0; i < 10; i++) {
    total = add(total, i);
  }
  while (total > MAX) {
    total = total - MAX;
  }
  if (total > 0) {
    printf("%s %d\n", msg, total);
  }
  return 0;
}

#define GREETING "hello { kept }"

struct Point {
  int x;
  int y;
};

/* distance helper: braces { } in comments are fine */
int dist(struct Point p) {
  int d = p.x + p.y;
  if (d < 0) {
    d = -d;
  }
  return d;
}

int sum_to(int n) {
  int total = 0;
  int i = 0;
  do {
    total = total + i;
    i++;
  } while (i <= n);
  switch (n) {
    case 0:
      return 0;
    default:
      break;
  }
  return total;
}
