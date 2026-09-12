// strings hold braces { like these } safely
public class Msgs {
  public String open() {
    return "{ hello }";
  }

  public void show() {
    String s = "a { b } c";
    /* block comment with { } inside */
    // line comment with } brace
    System.out.println(s);
  }
}
