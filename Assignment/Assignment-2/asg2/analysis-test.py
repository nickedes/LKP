from json import load
from subprocess import call

locks = {"NONE":0,"SPINLOCK" : 1,"RWLOCK" : 2,"SEQLOCK" : 3,"RCU" : 0,"RWLOCK_CUSTOM" : 5}

numthreads = 10
ops_per_thread = 5000000
readops = [99, 96, 92, 90, 87, 84, 80]

with open('data.json', 'r') as fp:
    results = load(fp)

for lock in results:
	totalTime = sum(results[lock].values())
	avgTime = totalTime/len(results[lock].values())
	variance = 0
	for val in results[lock].values():
		variance += (avgTime - val)**2

	variance = variance/len(results[lock].values())
	print(lock, ", Total time - ", totalTime, "Avg Time - ", avgTime, "Variance - ", variance)
