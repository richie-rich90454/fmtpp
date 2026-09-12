module main

import os

// a tiny adder
fn add(a int, b int) int {
    return a + b
}

struct Point {
    x f32
    y f32
}

fn main() {
    p := Point{ x: 1.0, y: 2.0 }
    println(add(1, 2))
}
