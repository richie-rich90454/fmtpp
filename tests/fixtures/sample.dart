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
