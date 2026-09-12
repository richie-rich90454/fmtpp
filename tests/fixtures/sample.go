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

// Scale returns v added to itself factor times.
func Scale(v float64, factor float64) float64 {
	total := 0.0
	for i := 0; i < int(factor); i++ {
		total = total + v
	}
	return total
}

// Label picks a name { switch below }.
func Label(n int) string {
	switch n {
	case 0:
		return "none"
	default:
		return "some"
	}
}
