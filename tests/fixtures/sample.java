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
