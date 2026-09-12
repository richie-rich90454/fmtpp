import java.util.List;
import java.util.ArrayList;

public class Dog extends Animal implements Pet {
  private String name;

  public Dog(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }

  public void bark() {
    switch (name) {
      case "Rex":
        System.out.println("Woof!");
        break;
      default:
        break;
    }
    try {
      fetch();
    } catch (Exception e) {
      throw e;
    } finally {
      cleanup();
    }
  }
}

// second helper class in the same file
class Cat extends Animal {
  private int lives = 9;

  public Cat(String name) {
    lives = 9;
  }

  // braces { } in comments stay put
  public boolean isLucky() {
    for (int i = 0; i < lives; i++) {
      if (i == 7) {
        return true;
      }
    }
    return false;
  }

  public String speak() {
    StringBuilder sb = new StringBuilder();
    int n = 0;
    while (n < 3) {
      sb.append("meow");
      n++;
    }
    return sb.toString();
  }
}
