// exercise every control form
def classify(n) {
    if (n < 0) {
        return -1
    } else if (n == 0) {
        return 0
    } else {
        return 1
    }
}

def run(xs) {
    def total = 0
    for (x in xs) {
        total += classify(x)
    }
    while (total > 4) {
        total -= 4
    }
    switch (total) {
        case 0:
            return 0
        default:
            break
    }
    try {
        check(total)
    } catch (Exception e) {
        return -1
    }
    return total
}
