// deep nesting stress
fn deep(n: i32) i32 {
    var r: i32 = 0;
    if (n > 0) {
        var i: i32 = 0;
        while (i < n) {
            while (r < i) {
                if (@rem(r, 2) == 0) {
                    var j: i32 = 0;
                    while (j < 2) {
                        if (j == i) {
                            r = r + 1;
                        } else {
                            r = r + 2;
                        }
                        j += 1;
                    }
                } else {
                    r = r + 1;
                }
            }
            i += 1;
        }
    }
    return r;
}
