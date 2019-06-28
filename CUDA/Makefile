serial_wave cuda_wave: serial_wave.c cuda_wave.cu
	gcc serial_wave.c -o serial_wave -lm
	nvcc cuda_wave.cu -o cuda_wave

serial:
	# /usr/bin/time -f "%e" ./serial_wave 5000 5000
	/usr/bin/time -f "%e" ./serial_wave 10000 10000
	# /usr/bin/time -f "%e" ./serial_wave 100000 100000
	# /usr/bin/time -f "%e" ./serial_wave 1000000 1000000

cuda:
	# /usr/bin/time -f "%e" ./cuda_wave 5000 5000
	/usr/bin/time -f "%e" ./cuda_wave 10000 10000
	# /usr/bin/time -f "%e" ./cuda_wave 100000 100000
	# /usr/bin/time -f "%e" ./cuda_wave 1000000 1000000

clean:
	rm serial_wave
	rm cuda_wave
