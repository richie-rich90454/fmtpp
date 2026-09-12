module myapp;

import std.stdio;

interface ILogger : IDisposable {
    void log(string message);
}

class ConsoleLogger : LoggerBase, ILogger {
    void log(string message) {
        writeln(message);
    }
}

void main() {
    auto logger = new ConsoleLogger();
    logger.log("hello");
    foreach (line; ["a", "b"]) {
        writeln(line);
    }
}
