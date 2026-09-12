module main

import os

struct Point {
    x f32
    y f32
}

enum Color {
    red
    green
    blue
}

fn add(a int, b int) int {
    return a + b
}

fn main() {
    p := Point{ x: 1.0, y: 2.0 }
    mut total := 0
    for i in 0 .. 10 {
        total += i
    }
    match total {
        0 { println('none') }
        else { println('other') }
    }
}
