# e2e-markers: "interp, %w[alpha, /re/im, :symbol
def strings(name)
  a = "double"
  b = 'single'
  c = "interp #{name} end"
  d = "nested #{outer("inner")} tail"
  e = 'no interp #{x}'
  f = "escapes \n \t \\ \" \'"
  g = %w[alpha beta]
  h = %w[then do end if while]
  i = %w[a=b;c]
  j = %w[<<EOS]
  k = %w[(nested)]
  l = %i[one two]
  m = %q{brace; inside}
  n = %r{a;b|c}im
  o = /re; gex/im
  p = %r{pat#{x}tern}
  q = "semi; inside"
  r = 'quote " mix'
  s = "quote ' mix"
  t = :symbol
  u = :symbol?
  v = :"quoted sym"
  w = "multi
line"
  x = 'concat' "mixed"
  y = %s<sym>
  z = %x{echo hi}
  back = `ls #{dir}`
  return z
end

defregex = /anchored/
