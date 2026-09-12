# e2e-markers: while2972, until done, for f in, times do
def loops(items, files)
  count = 0
  while count < 3 do
    count += 1
  end
  until done?
    work
  end
  for f in files
    puts f
  end
  n = 0
  n += 1 while n < 10
  process until finished?
  while true
    break
  end
end

def blocks(items)
  items.each do |x|
    puts x
  end
  hash.each do |k, v|
    puts "#{k}=#{v}"
  end
  outer do
    inner do |x|
      use(x)
    end
  end
  [1, 2].each { |i| puts i }
  y = [1, 2].map { |i|
    i * 2
  }.compact
  sq = 10.then do |v|
    v * v
  end
  looper do
    item = queue.pop
    redo if item.retry?
    next if item.skip?
    break if item.done?
    handle(item)
  end
end

BEGIN { setup }
END { teardown }
at_exit do
  bye
end
