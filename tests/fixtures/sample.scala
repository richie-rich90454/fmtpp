package example

import scala.math.PI

class Dog(val name: String) extends Animal {
    def speak(): String = {
        "Woof!"
    }
}

object Main {
    def add(a: Int, b: Int): Int = {
        a + b
    }

    def main(args: Array[String]): Unit = {
        val dog = new Dog("Rex")
        println(dog.speak())
        dog.name match {
            case "Rex" => println("good boy")
            case _ => println("who?")
        }
    }
}

// extra domain below
trait Pet {
    def speak(): String
}

case class Point(x: Int, y: Int)

object Stats {
    def total(xs: List[Int]): Int = {
        var sum = 0
        for (x <- xs) {
            sum += x
        }
        var left = sum
        while (left > 100) {
            left -= 100
        }
        left
    }

    // braces { } in comments stay put
    def label(n: Int): String = {
        if (n <= 0) "none" else "some"
    }
}
