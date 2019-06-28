all: pi
pi : pi.c
	gcc -pthread -std=gnu99 -O2 -s pi.c -o pi

# if you use c++ uncomment following two lines
#pi : pi.cpp
#   gcc -pthread -std=gnu99 -O2 -s pi.cpp -o pi

test:
	@time taskset -c 0 ./pi 1 100000000
	@time taskset -c 0,1 ./pi 2 100000000
	@time taskset -c 0,1,2,3 ./pi 4 100000000

test_mac:
	@time ./pi 1 100000000
	@time ./pi 2 100000000
	@time ./pi 4 100000000

# check the result
#3.141404
#0.62user 0.00system 0:00.62elapsed 99%CPU (0avgtext+0avgdata 1656maxresident)k
#0inputs+0outputs (0major+125minor)pagefaults 0swaps
#3.141707
#0.64user 0.00system 0:00.32elapsed 198%CPU (0avgtext+0avgdata 1620maxresident)k
#0inputs+0outputs (0major+128minor)pagefaults 0swaps
#3.141512
#0.68user 0.00system 0:00.17elapsed 379%CPU (0avgtext+0avgdata 1584maxresident)k                                                     
#0inputs+0outputs (0major+134minor)pagefaults 0swaps

clean:
	rm -rf pi *.o
