const std = @import("std");

// exercise every control form
fn classify(n: i32) i32 {
    if (n < 0) {
        return -1;
    } else if (n == 0) {
        return 0;
    } else {
        return 1;
    }
}

fn run(n: i32) i32 {
    var total: i32 = 0;
    var i: i32 = 0;
    while (i < n) {
        total += classify(i);
        i += 1;
    }
    for ([_]i32{ 1, 2 }) |v| {
        total += v;
    }
    switch (total) {
        0 => return 0,
        else => return total,
    }
}
