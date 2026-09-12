module main

// strings hold braces { like these } safely
fn open_msg() string {
    return '{ hello }'
}

fn main() {
    s := 'a { b } c'
    // line comment with } brace
    /* block comment with { } inside */
    println(s)
}
