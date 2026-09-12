package main

import "fmt"

// exercise every control form
func classify(n int) int {
	if n < 0 {
		return -1
	} else if n == 0 {
		return 0
	} else {
		return 1
	}
}

func run(xs []int) int {
	total := 0
	for _, x := range xs {
		total = total + classify(x)
	}
	for total > 4 {
		total = total - 4
	}
	switch total {
	case 0:
		return 0
	default:
		break
	}
	return total
}

func main() {
	fmt.Println(run([]int{1, 2}))
}
