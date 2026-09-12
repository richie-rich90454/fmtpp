// strings hold braces { like these } safely
const open_msg = "{ hello }";

function show(name) {
  const s = `a { b } ${name}`;
  /* block comment with { } inside */
  // line comment with } brace
  return s;
}

export { show, open_msg };
