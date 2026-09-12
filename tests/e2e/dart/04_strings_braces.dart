// strings hold braces { like these } safely
String openMsg() {
  return "{ hello }";
}

void main() {
  var s = "a { b } c";
  var t = 'x ${s}';
  // line comment with } brace
  /* block comment with { } inside */
  print(t);
}
