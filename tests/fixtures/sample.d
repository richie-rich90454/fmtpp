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

// Extra helpers below.
struct Point {
    int x;
    int y;
}

int total(int[] xs) {
    int sum = 0;
    foreach (x; xs) {
        sum += x;
    }
    while (sum > 100) {
        sum -= 100;
    }
    return sum;
}

string label(int n) {
    // braces { } in comments stay put
    switch (n) {
        case 0:
            return "none";
        default:
            return "some";
    }
}

int count_even(int[] xs) {
    int count = 0;
    foreach (x; xs) {
        if (x % 2 == 0) {
            count++;
        }
    }
    return count;
}
