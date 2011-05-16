#!/usr/bin/env ruby
#

x = [5,10,20,40]
seq = ['S', 'D', 'I']

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
	return iotime, totaltime
end

seq.each do |s|
	outFile = File.new("#{s}-lbd.txt", "w")
	x.each do |a|
		outFile.write "#{a}000 "
		folder = "../#{a}k.1/"

		file = "#{s}#{a}000A-1-NONE.log"
		iotime, totaltime = parse(folder+file)
		outFile.write "#{iotime} #{totaltime} "

		file = "#{s}#{a}000M-0-NONE.log"
		iotime, totaltime = parse(folder+file)
		outFile.write "#{iotime} #{totaltime} "

		file = "#{s}#{a}000D-1-NONE.log"
		iotime, totaltime = parse(folder+file)
		outFile.write "#{iotime} #{totaltime} "

		outFile.puts
	end
	outFile.close
	outFile = File.new("#{s}-lbd-sparse.txt", "w")
	x.each do |a|
		outFile.write "#{a}000 "
		folder = "../#{a}k.10/"

		file = "#{s}#{a}000A-1-NONE.log"
		iotime, totaltime = parse(folder+file)
		outFile.write "#{iotime} #{totaltime} "

		file = "#{s}#{a}000M-0-NONE.log"
		iotime, totaltime = parse(folder+file)
		outFile.write "#{iotime} #{totaltime} "

		file = "#{s}#{a}000D-1-NONE.log"
		iotime, totaltime = parse(folder+file)
		outFile.write "#{iotime} #{totaltime} "

		outFile.puts
	end
	outFile.close
end
