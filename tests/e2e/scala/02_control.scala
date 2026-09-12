package example

import scala.math.PI

// exercise every control form
object Loops {
    def classify(n: Int): Int = {
        if (n < 0) -1
        else if (n == 0) 0
        else 1
    }

    def run(xs: List[Int]): Int = {
        var total = 0
        for (x <- xs) {
            total += classify(x)
        }
        while (total > 4) {
            total -= 4
        }
        total match {
            case 0 => 0
            case _ => total
        }
    }
}
