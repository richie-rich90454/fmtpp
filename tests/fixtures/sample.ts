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
