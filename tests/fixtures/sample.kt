package com.example

import kotlin.math.PI

open class Animal(val name: String) {
    open fun speak(): String {
        return "..."
    }
}

class Dog(name: String) : Animal(name) {
    override fun speak(): String {
        return "Woof!"
    }
}

object Singleton {
    val answer = 42
}

fun add(a: Int, b: Int): Int {
    return a + b
}

fun main() {
    val dog = Dog("Rex")
    println(dog.speak())
    when (dog.name) {
        "Rex" -> println("good boy")
        else -> println("who?")
    }
}
