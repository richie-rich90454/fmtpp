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
