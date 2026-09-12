const std = @import("std");

// strings hold braces { like these } safely
fn openMsg() []const u8 {
    return "{ hello }";
}

pub fn main() void {
    const s = "a { b } c";
    // line comment with } brace
    // block note with { } inside
    std.debug.print("{s}", .{s});
    _ = openMsg;
}
