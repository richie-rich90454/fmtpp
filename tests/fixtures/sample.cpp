#include <iostream>
#include <string>

namespace shapes {

class Circle : public Shape {
 public:
  Circle(double radius) : radius_(radius) {}

  double area() { return 3.14 * radius_ * radius_; }

 private:
  double radius_;
};

int main() {
  Circle unit(1.0);
  std::cout << unit.area() << std::endl;
  return 0;
}

}  // namespace shapes

// free helpers below the namespace
int gcd(int a, int b) {
  while (b != 0) {
    int t = b;
    b = a % b;
    a = t;
  }
  return a;
}

class Counter {
 public:
  Counter() : value_(0) {}

  void add(int n) {
    for (int i = 0; i < n; i++) {
      value_++;
    }
  }

  int value() { return value_; }

 private:
  int value_;
};

// pick a unit circle by name { switch below }
shapes::Circle unit_circle() {
  shapes::Circle c(1.0);
  return c;
}
