// strings hold braces { like these } safely
fun openMsg(): String {
    return "{ hello }"
}

fun main() {
    val s = "a { b } c"
    val t = "x ${s}"
    // line comment with } brace
    /* block comment with { } inside */
    println(t)
}
