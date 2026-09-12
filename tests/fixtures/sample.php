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

// extra helpers below
interface Barker {
    public function bark(): string;
}

function total(array $xs): int {
    $sum = 0;
    foreach ($xs as $x) {
        $sum = $sum + $x;
    }
    while ($sum > 100) {
        $sum = $sum - 100;
    }
    return $sum;
}

function label(int $n): string {
    // braces { } in comments stay put
    switch ($n) {
        case 0:
            return "none";
        default:
            return "some";
    }
}

function count_even(array $xs): int {
    $count = 0;
    foreach ($xs as $x) {
        if ($x % 2 == 0) {
            $count++;
        }
    }
    return $count;
}
