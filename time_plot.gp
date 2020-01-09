reset
set ylabel 'time(sec)'
set xlabel 'input sorting'
set style fill solid
set title 'perfomance comparison'
set term png enhanced font 'Verdana,10'
set output 'time.png'

plot [:][:]'time_data.txt' using 2:xtic(1) with histogram title 'random input', \
'' using 3:xtic(1) with histogram title 'simulate input', \
'' using ($0-0.06):($2+0.001):2 with labels title ' ', \
'' using ($0+0.3):($3+0.0015):3 with labels title ' '