// strings hold braces { like these } safely
type Pair = [string, number];

const open_msg: string = "{ hello }";

function show(name: string): string {
  const s: string = `a { b } ${name}`;
  /* block comment with { } inside */
  // line comment with } brace
  return s;
}

export { show, open_msg };
export type { Pair };
