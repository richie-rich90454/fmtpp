import 'dart:math';

// a tiny circle
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
}
