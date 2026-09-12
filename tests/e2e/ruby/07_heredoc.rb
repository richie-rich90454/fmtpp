# e2e-markers: <<~EOS, <<-IND, __END__, =begin
y = foo(<<~EOS, 42)
  hello #{name};
  bye
EOS
puts y

q1 = <<-IND
  indented;
  body
  IND

q2 = <<"QUOTED;X"
  quoted; body
  QUOTED;X

w = <<ONE + <<TWO
first;
second;
ONE
third
TWO

stripped = get(<<~SQL).rows
  SELECT * FROM t
  WHERE x = 1
SQL

empty_doc = <<~EMPTY
EMPTY

=begin
Block comment here.
 spanning; lines with end inside words like weekend.
=end
puts "after begin block"
__END__
trailing data; here
=end not a marker anymore
