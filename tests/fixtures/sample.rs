use std::collections::HashMap;

mod shapes {
    pub struct Circle {
        radius: f64,
    }
}

trait Drawable {
    fn draw(&self);
}

impl Drawable for shapes::Circle {
    fn draw(&self) {}
}

#[derive(Debug)]
enum Color {
    Red,
    Green,
    Blue,
}

type Meters = f64;

fn area(radius: f64) -> f64 {
    let pi: f64 = 3.14;
    let mut total = 0.0;
    if radius <= 0.0 {
        panic!("bad radius");
    }
    for i in 0..10 {
        total = total + i as f64;
    }
    while total > 100.0 {
        total = total - 100.0;
    }
    match radius {
        _ => 0.0,
    }
}

fn main() -> i32 {
    let nums = vec![1, 2, 3];
    let add = |a, b| a + b;
    let name = format!("hello {}", "world");
    let maybe: Option<i32> = None;
    return 0;
}

const LIMIT: i32 = 100;

// Ratio helper with a loop and a comment { braces fine }.
fn ratio(a: f64, b: f64) -> f64 {
    if b == 0.0 {
        return 0.0;
    }
    let mut q = a / b;
    while q > LIMIT as f64 {
        q = q - LIMIT as f64;
    }
    return q;
}

struct Counter {
    value: i32,
}

impl Counter {
    fn add(&mut self, n: i32) {
        for _ in 0..n {
            self.value += 1;
        }
    }
}
