from subprocess import call
from time import sleep

locks = {"SPINLOCK" : 1,"RWLOCK" : 2,"SEQLOCK" : 3,"RCU" : 4,"RWLOCK_CUSTOM" : 5}

numthreads = 4
ops_per_thread = 50000
readops = [99, 96, 93, 90, 87, 84, 80, 77, 74, 70]

# Run Simple tests on locks

for lock in locks:
	print("Lock - " + lock)
	changelock = "echo "+ str(locks[lock]) +" > /sys/kernel/asg2_lock"
	print(changelock)
	call([changelock], shell=True)
	for readop in readops:
		# ./syncbench <numthreads> <ops/thread> <readops (%)> <writeops (%)>
		benchmarkCmd = "./syncbench " + str(numthreads) + " " + str(ops_per_thread) + " " + str(readop) + " " + str(100-readop)
		print(benchmarkCmd)
		call([benchmarkCmd], shell=True)
		sleep(1)
