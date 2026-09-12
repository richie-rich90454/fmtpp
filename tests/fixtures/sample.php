<?php
namespace App\Models;

use App\Contracts\Pet;

class Dog extends Animal implements Pet {
    private string $name;

    public function __construct(string $name) {
        $this->name = $name;
    }

    public function bark(): string {
        return "Woof!";
    }
}

function add(int $a, int $b): int {
    return $a + $b;
}
