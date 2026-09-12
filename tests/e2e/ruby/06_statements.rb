# e2e-markers: require "json", return value, raise "boom", throw :done
require "json"
require_relative "helper"
require("digest")

MAX = 100
$name = nil
@@count = 0
PI = 3.14

def flow(x)
  return if x.nil?
  return value if early?
  raise "boom" if bad?
  raise ArgumentError, "nope"
  throw :done
  super(x)
  super
  yield x
  defined?(Foo)
end

def calls
  puts 1, 2
  calc(1, foo: 2, *rest, &blk)
  obj.method(arg).chain
  aliased = original
end

alias greet hello
alias_method :shout, :greet
attr_accessor :x, :y
attr_reader :name
private :secret
protected
public
include Enumerable
extend self
catch(:done) { go }
fail "wheels off" if broken?
