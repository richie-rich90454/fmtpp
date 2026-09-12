// strings hold braces { like these } safely
fn open_msg() -> String {
    return "{ hello }".to_string();
}

fn main() {
    let s = "a { b } c";
    let t = format!("x {}", s);
    // line comment with } brace
    /* block comment with { } inside */
    println!("{}", t);
}
