package com.example

// a tiny dog
class Dog(val name: String) {
    fun speak(): String {
        return "Woof!"
    }
}

fun main() {
    val dog = Dog("Rex")
    println(dog.speak())
}
