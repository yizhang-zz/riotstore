#!/usr/bin/env ruby
#

seq = ['S', 'D', 'I']
dense = [0, 1]
h = {'S' => 'seq',
	'D' => 'str',
	'I' => 'int'
}
f = { 0 => 'sparse',
	1 => 'dynamic'
}

seq.each do |s|
	dense.each do |d|
		file = "#{s}#{d}"
		text = File.read("plot")
		IO.popen("gnuplot", "w+") { |io|
			tmp = text.gsub("$DATA$", file).gsub("$SEQ$", h[s]).gsub("$FORMAT$", f[d])
			io.write tmp
			io.close_write
		}
	end
end
