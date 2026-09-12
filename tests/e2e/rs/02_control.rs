// exercise every control form
fn classify(n: i32) -> i32 {
    if n < 0 {
        return -1;
    } else if n == 0 {
        return 0;
    } else {
        return 1;
    }
}

fn run(xs: Vec<i32>) -> i32 {
    let mut total = 0;
    for x in xs {
        total = total + classify(x);
    }
    while total > 4 {
        total = total - 4;
    }
    match total {
        0 => 0,
        _ => total,
    }
}
