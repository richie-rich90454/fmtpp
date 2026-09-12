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
