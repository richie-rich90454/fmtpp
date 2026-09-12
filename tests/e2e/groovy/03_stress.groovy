// deep nesting stress
def deep(n) {
    def r = 0
    if (n > 0) {
        for (i in 0..<n) {
            while (r < i) {
                if (r % 2 == 0) {
                    for (j in 0..<2) {
                        if (j == i) {
                            r = r + 1
                        } else {
                            r = r + 2
                        }
                    }
                } else {
                    r = r + 1
                }
            }
        }
    }
    return r
}
