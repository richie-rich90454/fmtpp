# e2e-markers: a + b, x ? y, foo &&, chain_A, 1...10
def operators(a, b)
  s = a + b
  d = a - b
  p = a * b
  q = a / b
  m = a % b
  e = a ** b
  c = a == b
  n = a != b
  lt = a < b
  gt = a > b
  le = a <= b
  ge = a >= b
  cmp = a <=> b
  rx = a =~ /x/
  nrx = a !~ /x/
  land = a && b
  lor = a || b
  kland = a and b
  klor = a or b
  neg = !a
  inv = ~a
  t = a ? b : c
  i = 0
  i += 1
  i -= 1
  i *= 2
  i /= 2
  i %= 2
  i **= 2
  i ||= 1
  i &&= 2
  return i
end

def continuations(foo, cond, a, b)
  doubled = foo +
    offset
  chained = foo.
    bar.
    baz
  called 1,
    2
  both = cond &&
    other
  either = cond ||
    fallback
  pick = cond ?
    a : b
  range = 1...10
  incl = 1..10
  shifted = x << 2
  appended = list << item
  return range
end

def calls_shapes(h)
  cmd_arg "text"
  cmd_sym :name
  cmd_hash a: 1, b: 2
  cmd_rocket :a => 1
  cmd_block { |x| x }
  cmd_paren(1, 2)
  no_args
  query?
  bang!
  chain_A.b_C.d?
  self.class::NAME
  h[:key]
  h["k"]
  arr[0]
  arr[1..2]
  arr[-1]
  s = -x
  t = +y
  u = *rest
  v = [1, *[2, 3]]
  w = { **opts }
  return w
end
