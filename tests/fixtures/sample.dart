import 'dart:math';

class Circle {
  double radius;

  Circle(this.radius);

  double area() {
    return pi * radius * radius;
  }
}

void main() {
  var c = Circle(2.0);
  print(c.area());
  for (var i = 0; i < 3; i++) {
    print(i);
  }
}

// extra shapes live here
enum Color {
  red,
  green,
}

class Counter {
  int value = 0;

  void add(int n) {
    for (var i = 0; i < n; i++) {
      value++;
    }
    while (value > 100) {
      value = value - 100;
    }
  }

  String label() {
    // braces { } in comments stay put
    switch (value) {
      case 0:
        return "none";
      default:
        return "some";
    }
  }

  int doubled() {
    return value * 2;
  }
}
