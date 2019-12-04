#!/usr/bin/ruby

require 'csv'

csv = CSV.read(ARGV[0], headers: true)

P0          = 1024.19   # 海面気圧
HEIGHT_BASE = 425.2     # 射点高度

max_height = 0.0
top_time = 0.0

csv.each do |data|
  id = data["id"].to_i
  time = data["time"].to_f
  temp = data["temperature"].to_f
  press= data["pressure"].to_f

  height = (((P0/press)**(1/1.257) - 1) * (temp + 273.15)) / 0.0065
  height = height - HEIGHT_BASE

  if height == Float::INFINITY
    next
  end
  if max_height < height
    max_height = height
    top_time = time
  end

  puts "#{id} #{time} #{temp} #{press} #{height}"
end

STDERR.puts "max height: #{max_height} (#{top_time} sec)"
