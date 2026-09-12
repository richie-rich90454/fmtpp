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
