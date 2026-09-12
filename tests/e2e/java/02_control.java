import java.util.List;

// exercise every control form
public class Loops {
  public int classify(int n) {
    if (n < 0) {
      return -1;
    } else if (n == 0) {
      return 0;
    } else {
      return 1;
    }
  }

  public int run(List<Integer> xs) {
    int total = 0;
    for (int x : xs) {
      total = total + classify(x);
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
      check(total);
    } catch (Exception e) {
      throw new RuntimeException(e);
    } finally {
      cleanup();
    }
    return total;
  }

  private void check(int n) {
  }

  private void cleanup() {
  }
}
