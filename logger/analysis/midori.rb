#!/usr/bin/ruby

require 'csv'

csv = CSV.read('midori.csv', headers: true)

P0 = 1024.19

max_height = 0.0
top_time = 0.0

csv.each do |data|
  id = data["id"].to_i
  time = data["time"].to_f + 365 - 2.5 + 1.65
  temp = data["temperature"].to_f
  press = data["pressure"].to_f
  ax = data["ax"].to_f
  ay = data["ay"].to_f
  az = data["az"].to_f
  acc2 = ax**2 + ay**2 + az**2
  acc = Math.sqrt(acc2);
  height = (((P0/press)**(1/5.257) - 1) *(temp + 273.15)) / 0.0065
  height = height - 425.2
  if height == Float::INFINITY
    next
  end
  if max_height < height
    max_height = height
    top_time = time
  end
  if id > 16220
    puts "#{time} #{temp} #{press} #{height} #{acc}"
  end
end

STDERR.puts "max height: #{max_height}(#{top_time} sec)"
