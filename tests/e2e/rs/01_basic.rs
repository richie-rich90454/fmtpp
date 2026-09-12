use std::collections::HashMap;

// a tiny adder
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

struct Point {
    x: f64,
    y: f64,
}

fn main() -> i32 {
    let p = Point { x: 1.0, y: 2.0 };
    return add(p.x as i32, p.y as i32);
}
