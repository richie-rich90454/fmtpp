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

// math helpers live here
const factor = 3;

function multiply(a, b) {
  let total = 0;
  for (let i = 0; i < b; i++) {
    total = total + a;
  }
  return total;
}

/* pick the larger value,
   braces { like this } stay in comments */
function max(a, b) {
  if (a > b) {
    return a;
  } else {
    return b;
  }
}

function describe(n) {
  switch (n) {
    case 0:
      return "none";
    default:
      return "some " + n;
  }
}

const label = "{ not a brace }";

function countdown(n) {
  while (n > 0) {
    n = n - 1;
  }
  return n;
}

try {
  countdown(factor);
} catch (e) {
  throw e;
}
