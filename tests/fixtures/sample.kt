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

// extra helpers below
data class Point(val x: Int, val y: Int)

interface Pet {
    fun speak(): String
}

fun total(xs: List<Int>): Int {
    var sum = 0
    for (x in xs) {
        sum += x
    }
    var left = sum
    while (left > 100) {
        left -= 100
    }
    return left
}

fun label(n: Int): String {
    // braces { } in comments stay put
    if (n <= 0) {
        return "none"
    }
    return "some $n"
}
