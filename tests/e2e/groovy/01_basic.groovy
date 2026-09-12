package example

import groovy.json.JsonSlurper

// a tiny dog
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
