#!/usr/bin/env ruby
#

seq = ['S', 'D', 'I']
h = {'S' => 'seq',
	'D' => 'str',
	'I' => 'int'
}

seq.each do |s|
		file = "#{s}-lbd"
		text = File.read("plot-lbd")
		IO.popen("gnuplot", "w+") { |io|
			tmp = text.gsub("$DATA$", file).gsub("$SEQ$", h[s]).gsub("$DENSE$", "dense matrix")
			io.write tmp
			io.close_write
		}
end

seq.each do |s|
		file = "#{s}-lbd-sparse"
		text = File.read("plot-lbd")
		IO.popen("gnuplot", "w+") { |io|
			tmp = text.gsub("$DATA$", file).gsub("$SEQ$", h[s]).gsub("$DENSE$", "dense matrix")
			io.write tmp
			io.close_write
		}
end
