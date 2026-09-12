#include <iostream>

// deep nesting stress
int deep(int n) {
  int r = 0;
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      while (r < i) {
        if (r % 2 == 0) {
          for (int j = 0; j < 2; j++) {
            if (j == i) {
              r = r + 1;
            } else {
              r = r + 2;
            }
          }
        } else {
          r = r + 1;
        }
      }
    }
  }
  return r;
}

int main() {
  std::cout << deep(3) << std::endl;
  return 0;
}
