#include <stdio.h>

// exercise every control form
int classify(int n) {
  if (n < 0) {
    return -1;
  } else if (n == 0) {
    return 0;
  } else {
    return 1;
  }
}

int main(void) {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    total = total + classify(i);
  }
  while (total > 5) {
    total = total - 5;
  }
  do {
    total++;
  } while (total < 0);
  switch (total) {
    case 0:
      printf("zero\n");
      break;
    default:
      printf("%d\n", total);
      break;
  }
  return 0;
}
