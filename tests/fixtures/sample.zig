const std = @import("std");

const Point = struct {
    x: f32,
    y: f32,
};

const Color = enum {
    red,
    green,
    blue,
};

pub fn add(a: i32, b: i32) i32 {
    return a + b;
}

pub fn main() void {
    const p = Point{ .x = 1.0, .y = 2.0 };
    var total: i32 = 0;
    for ([_]i32{ 1, 2, 3 }) |n| {
        total += n;
    }
    switch (total) {
        0 => std.debug.print("none", .{}),
        else => std.debug.print("some", .{}),
    }
}

// Extra helpers below.
const Limit: i32 = 100;

fn scale(v: f32, n: i32) f32 {
    var total: f32 = 0;
    var i: i32 = 0;
    while (i < n) {
        total += v;
        i += 1;
    }
    return total;
}

fn label(n: i32) []const u8 {
    // braces { } in comments stay put
    if (n <= 0) {
        return "none";
    }
    return "some";
}

const Zero: i32 = 0;

fn is_zero(n: i32) bool {
    if (n == Zero) {
        return true;
    }
    return false;
}
