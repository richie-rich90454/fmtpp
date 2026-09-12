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
