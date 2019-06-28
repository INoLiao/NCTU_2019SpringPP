histogram: histogram.cpp
	g++ -std=c++11 -o histogram histogram.cpp -lOpenCL

test_opencl:
	/usr/bin/time -f "%e" ./histogram img1.bmp
	/usr/bin/time -f "%e" ./histogram img2.bmp
	/usr/bin/time -f "%e" ./histogram img3.bmp
	/usr/bin/time -f "%e" ./histogram img4.bmp
	/usr/bin/time -f "%e" ./histogram img5.bmp
	/usr/bin/time -f "%e" ./histogram img6.bmp

serial:
	g++ -std=c++11 -o image-histogram image-histogram.cpp

test_serial:
	/usr/bin/time -f "%e" ./image-histogram img1.bmp
	/usr/bin/time -f "%e" ./image-histogram img2.bmp
	/usr/bin/time -f "%e" ./image-histogram img3.bmp
	/usr/bin/time -f "%e" ./image-histogram img4.bmp
	/usr/bin/time -f "%e" ./image-histogram img5.bmp
	/usr/bin/time -f "%e" ./image-histogram img6.bmp

test_all:
	/usr/bin/time -f "%e" ./histogram img1.bmp img2.bmp img3.bmp img4.bmp img5.bmp img6.bmp
	/usr/bin/time -f "%e" ./image-histogram img1.bmp img2.bmp img3.bmp img4.bmp img5.bmp img6.bmp

clean_img:
	rm hist_*.bmp
