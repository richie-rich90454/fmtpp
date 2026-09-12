package example

import groovy.json.JsonSlurper

class Dog {
    String name

    Dog(String name) {
        this.name = name
    }

    def bark() {
        println "Woof!"
    }
}

def add(a, b) {
    a + b
}

def main() {
    def dog = new Dog("Rex")
    println dog.name
}

// extra helpers below
trait Barker {
    abstract void bark()
}

def total(xs) {
    def sum = 0
    for (x in xs) {
        sum += x
    }
    while (sum > 100) {
        sum -= 100
    }
    return sum
}

def label(n) {
    // braces { } in comments stay put
    switch (n) {
        case 0:
            return "none"
        default:
            return "some"
    }
}
