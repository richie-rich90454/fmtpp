#include <stdio.h>

// strings hold braces { like these } safely
const char* open_msg(void) {
  return "{ hello }";
}

int main(void) {
  const char* s = "a { b } c";
  char q = '}';
  /* block comment with { } inside */
  // line comment with } brace
  printf("%s %c\n", s, q);
  return 0;
}
