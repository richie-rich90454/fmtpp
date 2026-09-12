// strings hold braces { like these } safely
func openMsg() -> String {
    return "{ hello }"
}

struct Box {
    let s: String
}

let b = Box(s: "a { b } c")
// line comment with } brace
/* block comment with { } inside */
print(b.s)
