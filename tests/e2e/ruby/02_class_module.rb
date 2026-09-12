# e2e-markers: class Dog, module Tricks, attr_reader, def initialize
class Dog < Animal
  attr_reader :name, :age

  def initialize(name, age = 0)
    @name = name
    @age = age
  end

  def bark(times = 1)
    times.times do |i|
      puts "#{@name} barks #{i}"
    end
    "woof"
  end

  private

  def secret
    42
  end
end

class << self
  def helper = :ok
end

class Outer::Inner < Base
end

module Tricks
  def sit
    "sitting"
  end
end

module_function

def after_module_function
  1
end
