module main

// exercise every control form
fn classify(n int) int {
    if n < 0 {
        return -1
    } else if n == 0 {
        return 0
    } else {
        return 1
    }
}

fn run(n int) int {
    mut total := 0
    for i in 0 .. n {
        total += classify(i)
    }
    for total > 4 {
        total -= 4
    }
    match total {
        0 { return 0 }
        else { return total }
    }
}
