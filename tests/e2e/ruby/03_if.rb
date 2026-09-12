# e2e-markers: if tired, elsif hungry, unless ready, ternary
def decide(state)
  if state == "go"
    launch
  elsif state == "slow" || !state
    queue
  else
    abort
  end
end

def checks(a, b, ready, quiet)
  puts "hi" if ready
  puts "shh" unless quiet
  x = tired? ? rest : work
  y = defined?(Foo) ? 1 : 0
  if a then b end
  unless ready?
    wait
  end
end

def nested(a, b, c)
  if a
    if b
      return 1 if c
    else
      return 2
    end
  else
    return 3
  end
end

def guards(x)
  if x.is_a?(Integer) && x > 0
    positive
  end
  if !x.nil? && !x.empty?
    present
  end
end
