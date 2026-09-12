const std = @import("std");

fn broken( {
    return 1;
}

pub fn main() void {
    var x = = 5;
    if x > 0
        std.debug.print("hi", .{});
    else
        orphan();
}
