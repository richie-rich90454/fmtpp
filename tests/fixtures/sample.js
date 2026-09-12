import { useState } from "react";

const numbers = [1, 2, 3];

function add(a, b) {
  return a + b;
}

const double = (n) => n * 2;

class Counter {
  constructor() {
    this.count = 0;
  }
}

export { add, double, Counter };
export default function main() {
  return 0;
}
