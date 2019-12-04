#!/usr/bin/ruby

require 'csv'

csv = CSV.read('engine.csv', headers: true)

P0 = 1013.0

csv.each do |data|
  time = data["time"].to_f
  temp = data["temperature"].to_f
  press = data["pressure"].to_f
  height = (((P0/press)**(1/5.257) - 1) *(temp + 273.15)) / 0.0065
  if height > 30.0
    puts "#{time} #{temp} #{press} #{height}"
  end
end
