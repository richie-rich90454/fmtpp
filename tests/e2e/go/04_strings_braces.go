package main

import "fmt"

// strings hold braces { like these } safely
func openMsg() string {
	return "{ hello }"
}

func main() {
	s := "a { b } c"
	// line comment with } brace
	/* block comment with { } inside */
	fmt.Println(s)
}
