package main

import "fmt"

// a tiny adder
func add(a int, b int) int {
	return a + b
}

func main() {
	fmt.Println(add(1, 2))
}
