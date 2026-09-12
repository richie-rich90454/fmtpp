package main

import "fmt"

type Shape struct {
	Name string
}

type Drawer interface {
	Draw() string
}

const Pi = 3.14

const (
	ColorUndefined = iota
	Red
	Green
	Blue
)

type Meters float64

func area(radius float64) float64 {
	if radius <= 0 {
		panic("bad radius")
	}
	return Pi * radius * radius
}

func main() {
	size := 42
	for i := 0; i < size; i++ {
		fmt.Println(i)
	}
	for ok {
		break
	}
}
