type StringOrNumber = string | number;

enum Color {
  Red,
  Green,
  Blue,
}

interface Person {
  name: string;
  age: number;
}

interface Employee extends Person {
  salary: number;
}

function add(a: number, b: number): number {
  return a + b;
}

export { add, Color };
export type { Person, Employee, StringOrNumber };

// extra domain types below
type ID = string | number;

interface Named {
  id: ID;
  label: string;
}

class Registry implements Named {
  id: ID = 0;
  label = "main";

  lookup(id: ID): string {
    if (id === this.id) {
      return this.label;
    }
    return "unknown";
  }
}

function repeat(word: string, times: number): string {
  let out = "";
  for (let i = 0; i < times; i++) {
    out = out + word;
  }
  return out;
}

enum Size {
  Small,
  Large,
}

function sizeName(s: Size): string {
  switch (s) {
    case Size.Small:
      return "small";
    default:
      return "large";
  }
}

// braces { } in comments stay put
const tag = "{ kept in string }";

function countdown(n: number): number {
  while (n > 0) {
    n = n - 1;
  }
  return n;
}
