using System;

// a simple greeter
public class Greeter {
  private string name;

  public Greeter(string name) {
    this.name = name;
  }

  public string Greet() {
    return "hello " + name;
  }
}

public class Program {
  public static void Main() {
    Greeter g = new Greeter("Rex");
    Console.WriteLine(g.Greet());
  }
}
