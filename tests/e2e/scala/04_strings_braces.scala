// strings hold braces { like these } safely
object Msgs {
    def open(): String = {
        "{ hello }"
    }

    def show(name: String): String = {
        val s = s"a { b } $name"
        // line comment with } brace
        /* block comment with { } inside */
        s
    }
}
