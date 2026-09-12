// strings hold braces { like these } safely
def openMsg() {
    "{ hello }"
}

def show(name) {
    def s = "a { b } $name"
    // line comment with } brace
    /* block comment with { } inside */
    println s
    return s
}
