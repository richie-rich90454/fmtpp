#include <string>

// strings hold braces { like these } safely
std::string open_msg() {
  return "{ hello }";
}

int main() {
  std::string s = "a { b } c";
  /* block comment with { } inside */
  // line comment with } brace
  return (int)s.size();
}
