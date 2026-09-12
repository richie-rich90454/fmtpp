import Foundation

// a tiny dog
class Dog {
    let name: String

    init(name: String) {
        self.name = name
    }

    func bark() -> String {
        return "Woof!"
    }
}

let dog = Dog(name: "Rex")
print(dog.bark())
