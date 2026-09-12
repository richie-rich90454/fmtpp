#include <stdio.h>

// add two numbers
int add(int a, int b) {
  return a + b;
}

struct Box {
  int w;
  int h;
};

int main(void) {
  struct Box b = {3, 4};
  printf("%d\n", add(b.w, b.h));
  return 0;
}
