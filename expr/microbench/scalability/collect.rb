#!/usr/bin/env ruby
#

x = [5,10,20,40]
seq = ['S', 'D', 'I']
dense = [0, 1]
methods = ['M', 'A', 'R', 'T']

$base = 0

# columns in each generated file:
#   matrix size
#   IO time for strategy A
#   total time for strategy A
#   IO time for ...
#   total time for ...
#

def parse(file)
	line = %x[tail -n 1 #{file}]
	fields = line.split(' ')
	iotime = fields[3].to_i + fields[4].to_i
	totaltime = fields[5].to_i
	if $base == 0
		$base = totaltime.to_f
	end
	return iotime, totaltime/$base
end

dense.each do |d|
	seq.each do |s|
		outFile = File.new("#{s}#{d}.txt", "w")
		x.each do |a|
			$base = 0
			outFile.write "#{a}000 "
			folder = "../#{a}k-/"
			methods.each do |m|
				file = "#{s}#{a}000#{m}-#{d}-NONE.log"
				iotime, totaltime = parse(folder+file)
				outFile.write "#{iotime} #{totaltime} "
			end
			outFile.puts
		end
		outFile.close
	end
end
