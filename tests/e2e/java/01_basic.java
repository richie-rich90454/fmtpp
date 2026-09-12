import java.util.List;
import java.util.ArrayList;

// a simple dog
public class Dog extends Animal {
  private String name;

  public Dog(String name) {
    this.name = name;
  }

  public String getName() {
    return name;
  }

  public void bark() {
    System.out.println("Woof!");
  }
}
