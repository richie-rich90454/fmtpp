package com.example

// exercise every control form
fun classify(n: Int): Int {
    if (n < 0) {
        return -1
    } else if (n == 0) {
        return 0
    } else {
        return 1
    }
}

fun run(xs: List<Int>): Int {
    var total = 0
    for (x in xs) {
        total += classify(x)
    }
    while (total > 4) {
        total -= 4
    }
    when (total) {
        0 -> return 0
        else -> return total
    }
}
