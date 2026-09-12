const std = @import("std");

// a tiny adder
pub fn add(a: i32, b: i32) i32 {
    return a + b;
}

const Point = struct {
    x: f32,
    y: f32,
};

pub fn main() void {
    const p = Point{ .x = 1.0, .y = 2.0 };
    std.debug.print("{}", .{add(1, 2)});
    _ = p;
}
