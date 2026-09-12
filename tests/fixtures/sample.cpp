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
