using System;

public class Broken( {
  public int Value() {
    return 1;
  }
}

public class Program {
  public static void Main() {
    int x = = 5;
    if x > 0
      Console.WriteLine("hi");
    else
      orphan_call();
  }
}
