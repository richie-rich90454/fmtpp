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
