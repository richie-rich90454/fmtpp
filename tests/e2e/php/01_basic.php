<?php
namespace App;

// a tiny adder
function add(int $a, int $b): int {
    return $a + $b;
}

class Counter {
    private int $value = 0;

    public function bump(): int {
        $this->value++;
        return $this->value;
    }
}
