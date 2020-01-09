REPEAT = 1
RANDOM_DATA = random_command.txt
SIMULATE_DATA = simulate_command.txt
GOOGLE_DATA = google_command.txt

PERF_STAT = perf stat -o tem_output.txt --repeat $(REPEAT) 

CYCLE_GREP = grep 'cycles' tem_output.txt | tr -d ',' | grep -Eo '[0-9]+' | tr -d '\n' >> cycles_data.txt
CACHE_GREP = grep 'cache-misses' tem_output.txt | tr -d ',' | grep -Eo '[0-9]+' | tr -d '\n' >> cache-misses_data.txt
TIME_GREP = grep 'elapsed' tem_output.txt | tr -d ',' | sed "s/\+.*//" | grep -Eo '[0-9]+\.[0-9]+' | tr -d '\n' >> time_data.txt


all:qf.c bloom.c tst.c test_common.c input_generator.c
	gcc -c qf.c -O2
	ar rcs libqf.a qf.o

	gcc -c bloom.c -O2
	ar rcs libbloom.a bloom.o

	gcc -c tst.c -O2
	ar rcs libtst.a tst.o

	gcc -D REFMODE test_common.c -o test_common -L. -lqf -lbloom -ltst -lm -O0
	
	gcc input_generator.c -o input_generator


qf:qf.c
	gcc -c qf.c -O2
	ar rcs libqf.a qf.o

bloom:bloom.c
	gcc -c bloom.c -O2
	ar rcs libbloom.a bloom.o

tst:tst.c
	gcc -c tst.c -O2
	ar rcs libtst.a tst.o
    
bench:test_common
	./test_common --bench $(RANDOM_DATA)
	
input_gen:input_generator.c
	gcc input_generator.c -o input_generator
			  
cache-misses_random:test_common
	
	echo -n "original "> cache-misses_data.txt

	$(PERF_STAT) -e cache-misses ./test_common cities.txt --bench $(RANDOM_DATA)
	$(CACHE_GREP)
	echo "">> cache-misses_data.txt
	
	
	
	echo -n "dictionary ">> cache-misses_data.txt
	
	$(PERF_STAT) -e cache-misses ./test_common city_one_col.txt --bench $(RANDOM_DATA)
	$(CACHE_GREP)
	echo "">> cache-misses_data.txt
	
	
	
	echo -n "prefix(2) ">> cache-misses_data.txt
	
	$(PERF_STAT) -e cache-misses ./test_common city_two_prefix.txt --bench $(RANDOM_DATA)
	$(CACHE_GREP)
	echo "">> cache-misses_data.txt


	echo -n "prefix(3) ">> cache-misses_data.txt
	
	$(PERF_STAT) -e cache-misses ./test_common city_three_prefix.txt --bench $(RANDOM_DATA)
	$(CACHE_GREP)
	echo "">> cache-misses_data.txt
	
	
	gnuplot cache_plot.gp
	
	
cache-misses_simulate:test_common
	
	echo -n "original "> cache-misses_data.txt

	$(PERF_STAT) -e cache-misses ./test_common cities.txt --bench $(SIMULATE_DATA)
	$(CACHE_GREP)
	echo "">> cache-misses_data.txt
	
		
	echo -n "dictionary ">> cache-misses_data.txt
	
	$(PERF_STAT) -e cache-misses ./test_common city_one_col.txt --bench $(SIMULATE_DATA)
	$(CACHE_GREP)
	echo "">> cache-misses_data.txt
	
	
	
	echo -n "prefix(2) ">> cache-misses_data.txt
	
	$(PERF_STAT) -e cache-misses ./test_common city_two_prefix.txt --bench $(SIMULATE_DATA)
	$(CACHE_GREP)
	echo "">> cache-misses_data.txt


	echo -n "prefix(3) ">> cache-misses_data.txt
	
	$(PERF_STAT) -e cache-misses ./test_common city_three_prefix.txt --bench $(SIMULATE_DATA)
	$(CACHE_GREP)
	echo "">> cache-misses_data.txt
	
	
	gnuplot cache_plot.gp

	
cycles:test_common

	echo -n "original "> cycles_data.csv

	$(PERF_STAT) -e cycles ./test_common cities.txt --bench $(RANDOM_DATA)
	$(CYCLE_GREP)
	echo -n " ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common cities.txt --bench $(SIMULATE_DATA)
	$(CYCLE_GREP)
	echo "">> cycles_data.csv
	
	
	
	echo -n "dictionary ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common city_one_col.txt --bench $(RANDOM_DATA)
	$(CYCLE_GREP)
	echo -n " ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common city_one_col.txt --bench $(SIMULATE_DATA)
	$(CYCLE_GREP)
	echo "">> cycles_data.csv
	
	
	
	echo -n "prefix(2) ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common city_two_prefix.txt --bench $(RANDOM_DATA)
	$(CYCLE_GREP)
	echo -n " ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common city_two_prefix.txt --bench $(SIMULATE_DATA)
	$(CYCLE_GREP)
	echo "">> cycles_data.csv



	echo -n "prefix(3) ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common city_three_prefix.txt --bench $(RANDOM_DATA)
	$(CYCLE_GREP)
	echo -n " ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common city_three_prefix.txt --bench $(SIMULATE_DATA)
	$(CYCLE_GREP)
	echo "">> cycles_data.csv
	
	
	
	echo -n "mini-data-set ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common city_three_prefix.txt --bench $(RANDOM_DATA)
	$(CYCLE_GREP)
	echo -n " ">> cycles_data.csv
	
	$(PERF_STAT) -e cycles ./test_common city_three_prefix.txt --bench $(SIMULATE_DATA)
	$(CYCLE_GREP)
	echo "">> cycles_data.csv	
	
	

time_simulate:test_common

	echo -n "original "> time_data.txt
	$(PERF_STAT)\
    ./test_common cities.txt --bench $(SIMULATE_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt
	
	
	echo -n "dictionary ">> time_data.txt	
	$(PERF_STAT)\
    ./test_common city_one_col.txt --bench $(SIMULATE_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt
	
	
	echo -n "prefix(2) ">> time_data.txt
	$(PERF_STAT)\
    ./test_common city_two_prefix.txt --bench $(SIMULATE_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt


	echo -n "prefix(3) ">> time_data.txt
	$(PERF_STAT)\
    ./test_common city_three_prefix.txt --bench $(SIMULATE_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt
	
	
	gnuplot time_plot.gp
	
time_random:test_common

	echo -n "original "> time_data.txt

	$(PERF_STAT)\
	./test_common cities.txt --bench $(RANDOM_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt
	
	
	echo -n "dictionary ">> time_data.txt
	
	$(PERF_STAT)\
    ./test_common city_one_col.txt --bench $(RANDOM_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt
	
	
	echo -n "prefix(2) ">> time_data.txt
	
	$(PERF_STAT)\
    ./test_common city_two_prefix.txt --bench $(RANDOM_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt
	
	echo -n "two2(2) ">> time_data.txt
	
	$(PERF_STAT)\
    ./test_common two2.txt --bench $(RANDOM_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt


	echo -n "prefix(3) ">> time_data.txt
	
	$(PERF_STAT)\
    ./test_common city_three_prefix.txt --bench $(RANDOM_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt
	
	echo -n "three2(2) ">> time_data.txt
	
	$(PERF_STAT)\
    ./test_common three2.txt --bench $(RANDOM_DATA)
	$(TIME_GREP)
	echo "">> time_data.txt
	
	
	gnuplot time_plot.gp
	
		
	
	
	  