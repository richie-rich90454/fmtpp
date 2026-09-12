import Foundation

struct Point: Equatable {
    var x: Double
    var y: Double
}

class Dog: Animal {
    let name: String

    init(name: String) {
        self.name = name
    }

    func bark() -> String {
        return "Woof!"
    }
}

func add(_ a: Int, _ b: Int) -> Int {
    return a + b
}

let pi = 3.14
for x in 0..<10 {
    print(x)
}
switch pi {
case 3.14:
    print("pi")
default:
    break
}

// helpers below
enum Size: Int {
    case small = 0
    case large = 1
}

func total(_ xs: [Int]) -> Int {
    var sum = 0
    for x in xs {
        sum += x
    }
    var left = sum
    while left > 100 {
        left -= 100
    }
    return left
}

func label(_ n: Int) -> String {
    // braces { } inside comments are fine
    if n <= 0 {
        return "none"
    }
    return "some"
}
