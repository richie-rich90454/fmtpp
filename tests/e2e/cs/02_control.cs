using System;

namespace MyApp {

// exercise every control form
public class Loops {
  public int Classify(int n) {
    if (n < 0) {
      return -1;
    } else if (n == 0) {
      return 0;
    } else {
      return 1;
    }
  }

  public int Run() {
    int total = 0;
    for (int i = 0; i < 8; i++) {
      total = total + Classify(i);
    }
    while (total > 4) {
      total = total - 4;
    }
    switch (total) {
      case 0:
        return 0;
      default:
        break;
    }
    try {
      Check(total);
    } catch (Exception) {
      return -1;
    } finally {
      Cleanup();
    }
    return total;
  }

  private void Check(int n) {
  }

  private void Cleanup() {
  }
}

}
