require "json"

class Dog < Animal
  attr_reader :name

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
end

def fetch(path)
  data = File.read(path)
  JSON.parse(data)
rescue JSON::ParserError => e
  warn e.message
  nil
ensure
  log("done")
end

def classify(cmd)
  case cmd.split
  in ["go", target] then "going to #{target}"
  in ["stop"] then "stopping"
  else "unknown"
  end
end

if __FILE__ == $PROGRAM_NAME
  dog = Dog.new("Rex", 3)
  puts dog.bark(2)
end

# geometry helpers below
def area(width, height = 1)
  total = 0
  height.times do
    total = total + width
  end
  while total > 100
    total = total - 100
  end
  total
end

class Counter
  # braces { } in comments stay put
  def initialize
    @value = 0
  end

  def add(n)
    return @value if n <= 0
    @value = @value + n
  end
end
