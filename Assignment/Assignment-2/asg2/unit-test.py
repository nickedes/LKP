from json import dump
from subprocess import call
from time import time

locks = {"SPINLOCK" : 1,"RWLOCK" : 2,"SEQLOCK" : 3,"RCU" : 4,"RWLOCK_CUSTOM" : 5}

numthreads = 8
ops_per_thread = 5000000
readops = [99, 96, 93, 90, 87, 84, 80, 77, 74, 70]

results = {}

for lock in locks:
	results[lock] = {}

for lock in locks:
	print("Lock - " + lock)
	changelock = "echo "+ str(locks[lock]) +" > /sys/kernel/asg2_lock"
	print(changelock)
	call([changelock], shell=True)
	for readop in readops:
		# ./syncbench <numthreads> <ops/thread> <readops (%)> <writeops (%)>
		benchmarkCmd = "./syncbench " + str(numthreads) + " " + str(ops_per_thread) + " " + str(readop) + " " + str(100-readop)
		print(benchmarkCmd)
		start = time()
		call([benchmarkCmd], shell=True)
		end = time()
		results[lock][readop] = end - start
		with open('data.json', 'w') as fp:
			dump(results, fp, sort_keys=True, indent=4)


# all results stored here!
with open('data.json', 'w') as fp:
	dump(results, fp, sort_keys=True, indent=4)
