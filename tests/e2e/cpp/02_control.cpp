#include <iostream>

namespace shapes {

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

int run() {
  int total = 0;
  for (int i = 0; i < 8; i++) {
    total = total + classify(i);
  }
  while (total > 4) {
    total = total - 4;
  }
  switch (total) {
    case 0:
      std::cout << "zero" << std::endl;
      break;
    default:
      std::cout << total << std::endl;
      break;
  }
  try {
    if (total < 0) {
      throw total;
    }
  } catch (int e) {
    std::cout << e << std::endl;
  }
  return total;
}

}  // namespace shapes
