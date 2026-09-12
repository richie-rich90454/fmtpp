using System;

// strings hold braces { like these } safely
public class Msgs {
  public string Open() {
    return "{ hello }";
  }

  public void Show() {
    string s = "a { b } c";
    /* block comment with { } inside */
    // line comment with } brace
    Console.WriteLine(s);
  }
}
