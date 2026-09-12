using System;
using System.Collections.Generic;

namespace MyApp {

public interface ILogger : IDisposable {
  void Log(string message);
}

public class ConsoleLogger : LoggerBase, ILogger {
  public void Log(string message) {
    Console.WriteLine(message);
  }

  public void Dispose() {
  }
}

}

// extra domain code lives here
public enum Color {
  Red,
  Green,
}

public class Counter {
  private int value;

  public void Add(int n) {
    for (int i = 0; i < n; i++) {
      value++;
    }
  }

  public int Value() {
    switch (value) {
      case 0:
        return 0;
      default:
        return value;
    }
  }

  public override string ToString() {
    // braces { } inside comments are fine
    try {
      return Value().ToString();
    } catch (Exception) {
      return "bad";
    }
  }
}
