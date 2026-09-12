#!/usr/bin/env ruby
# frozen_string_literal: true
# e2e-markers: frozen_string_literal, trailing here, =begin, __END__

# leading comment
x = 1  # trailing here


# comment after blank
def f
  # comment opening suite
  y = 2
  # comment closing suite
end
# comment after end


if a
  b
  # comment before else
else
  c
end

case v
when 1
  # comment in arm
  one
  # comment closing arm
when 2 then two # trailing on arm
end

begin
  go
rescue
  # comment in rescue
  fix
end

items.each do |i| # trailing on opener
  # comment opening block
  puts i
end

=begin
Block comment.
=end
__END__
data tail here
