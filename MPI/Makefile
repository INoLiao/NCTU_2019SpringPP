conduction conduction_serial: conduction.c conduction_serial.c
	gcc conduction_serial.c -o conduction_serial
	mpicc conduction.c -o conduction

conduction_bk: conduction_bk.c
	mpicc conduction_bk.c -o conduction_bk

conduction_bk2: conduction_bk2.c
	mpicc conduction_bk.c -o conduction_bk2

test_serial_32:
	/usr/bin/time -f "%e" ./conduction_serial 32 2

test_serial_64:
	/usr/bin/time -f "%e" ./conduction_serial 64 2

test_serial_192:
	/usr/bin/time -f "%e" ./conduction_serial 192 2

test_192:
	/usr/bin/time -f "%e" mpiexec -n 2 --map-by node --hostfile hostfile conduction 192 2
	/usr/bin/time -f "%e" mpiexec -n 3 --map-by node --hostfile hostfile conduction 192 2
	/usr/bin/time -f "%e" mpiexec -n 4 --map-by node --hostfile hostfile conduction 192 2

test_32:
	/usr/bin/time -f "%e" mpiexec -n 2 --map-by node --hostfile hostfile conduction 32 2
	/usr/bin/time -f "%e" mpiexec -n 3 --map-by node --hostfile hostfile conduction 32 2
	/usr/bin/time -f "%e" mpiexec -n 4 --map-by node --hostfile hostfile conduction 32 2

test_64:
	/usr/bin/time -f "%e" mpiexec -n 2 --map-by node --hostfile hostfile conduction 64 2
	/usr/bin/time -f "%e" mpiexec -n 3 --map-by node --hostfile hostfile conduction 64 2
	/usr/bin/time -f "%e" mpiexec -n 4 --map-by node --hostfile hostfile conduction 64 2

execute:
	/usr/bin/time -f "%e" mpiexec -npernode 1 --hostfile hostfile conduction 48 1
	/usr/bin/time -f "%e" mpiexec -npernode 2 --hostfile hostfile conduction 48 1
	/usr/bin/time -f "%e" mpiexec -npernode 3 --hostfile hostfile conduction 48 1
	/usr/bin/time -f "%e" mpiexec -npernode 4 --hostfile hostfile conduction 48 1

time:
	/usr/bin/time -f "%e" mpiexec -npernode 1 --hostfile hostfile conduction 32 1
	/usr/bin/time -f "%e" mpiexec -npernode 2 --hostfile hostfile conduction 32 1
	/usr/bin/time -f "%e" mpiexec -npernode 3 --hostfile hostfile conduction 32 1
	/usr/bin/time -f "%e" mpiexec -npernode 4 --hostfile hostfile conduction 32 1

clean:
	rm conduction
	rm conduction_serial

test_bk_192:
	/usr/bin/time -f "%e" mpiexec -n 2 --map-by node --hostfile hostfile conduction_bk 192 2
	/usr/bin/time -f "%e" mpiexec -n 3 --map-by node --hostfile hostfile conduction_bk 192 2
	/usr/bin/time -f "%e" mpiexec -n 4 --map-by node --hostfile hostfile conduction_bk 192 2

test_bk2_192:
	/usr/bin/time -f "%e" mpiexec -n 2 --map-by node --hostfile hostfile conduction_bk2 192 2
	/usr/bin/time -f "%e" mpiexec -n 3 --map-by node --hostfile hostfile conduction_bk2 192 2
	/usr/bin/time -f "%e" mpiexec -n 4 --map-by node --hostfile hostfile conduction_bk2 192 2
