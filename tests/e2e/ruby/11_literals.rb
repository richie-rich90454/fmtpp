# e2e-markers: 42, 0xFF, 1..3, { a: 1 }, nil
def literals
  a = 42
  b = 3.14
  c = 0xFF
  d = 0o17
  e = 0b101
  f = 1_000
  g = nil
  h = true
  i = false
  arr = [1, 2, 3]
  nested = [[1], [2, [3]]]
  multi = [
    1,
    2,
  ]
  hsh = { a: 1, "b" => 2 }
  nested_h = { x: { y: [1] } }
  r = 1..3
  re = 1...3
  syms = [:a, :b]
  return arr
end
