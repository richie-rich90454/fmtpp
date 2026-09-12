# e2e-markers: case cmd, when 1, in [a, b], begin, rescue E, ensure
def classify(cmd)
  case cmd
  when 1, 2 then low
  when 3
    mid
  else
    high
  end
end

def match_patterns(data)
  case data
  in { name: } then puts name
  in [a, b] if a > b
    puts a
  in Integer => n
    puts n
  else
    puts "other"
  end
end

def bare_case(x)
  case
  when x > 1
    big
  end
end

def guarded
  begin
    attempt
  rescue IOError => e
    warn e.message
  rescue StandardError
    raise
  else
    recovered
  ensure
    cleanup
  end
end

def method_level(path)
  data = File.read(path)
  parse(data)
rescue ParseError => e
  log(e)
  nil
else
  puts "ok"
ensure
  close
end

def only_ensure
  work
ensure
  always
end
