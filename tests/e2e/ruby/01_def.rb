# e2e-markers: def greet, def self.version, def endless, endless endless
def greet(name, greeting = "hello", *args, loud: false, **kw)
  "#{greeting}, #{name}"
end

def self.version
  "1.0"
end

def endless = 42

def computed(x) = x * 2

def setter=(v)
  @v = v
end

def empty
end

def one_liner; 42; end

def with_block_param(a, &blk)
  blk.call(a) if blk
end

def nested
  helper = lambda do |x|
    x + 1
  end
  helper.call(1)
end
