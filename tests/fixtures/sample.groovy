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
