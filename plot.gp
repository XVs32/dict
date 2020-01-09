reset
set ylabel 'time(sec)'
set xlabel 'input sorting'
set style fill solid
set title 'perfomance comparison'
set term png enhanced font 'Verdana,10'
set output 'runtime.png'

plot [:][:]'output.txt' using 2:xtic(1) with histogram title 'random', \
'' using 3:xtic(1) with histogram title 'error input'  , \
'' using 4:xtic(1) with histogram title 'search trends'