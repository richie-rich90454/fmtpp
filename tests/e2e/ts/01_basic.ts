interface Person {
  name: string;
  age: number;
}

// a tiny greeter
function greet(p: Person): string {
  return "hello " + p.name;
}

export { greet };
export type { Person };
