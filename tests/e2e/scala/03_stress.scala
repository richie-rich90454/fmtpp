// deep nesting stress
object Deep {
    def run(n: Int): Int = {
        var r = 0
        if (n > 0) {
            for (i <- 0 until n) {
                while (r < i) {
                    if (r % 2 == 0) {
                        for (j <- 0 until 2) {
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
        r
    }
}
