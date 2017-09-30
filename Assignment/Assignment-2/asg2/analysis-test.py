from json import load
from subprocess import call

def plot(l, mean, sd):
	import matplotlib.pyplot as plt
	# sample
	plt.errorbar(l, mean, sd, linestyle='None', marker='o')
	plt.show()

locks = {"SPINLOCK" : 1,"RWLOCK" : 2,"SEQLOCK" : 3,"RCU" : 4,"RWLOCK_CUSTOM" : 5}

numthreads = 10
ops_per_thread = 5000000

results = {}

for lock in locks:
	results[lock] = {}
 
with open('data.json', 'r') as fp:
    results = load(fp)

mean = [0]*len(locks)
sd = [0]*len(locks)
l = [0]*len(locks)

for lock in results:
	totalTime = sum(results[lock].values())
	avgTime = totalTime/len(results[lock].values())
	variance = 0
	for val in results[lock].values():
		variance += (avgTime - val)**2

	variance = variance/len(results[lock].values())
	std_dev = variance**0.5
	print(lock, ", Total time - ", totalTime, "Avg Time - ", avgTime, "Variance - ", variance, "Standard Deviation - ", std_dev)
	mean[locks[lock]] = avgTime
	l[locks[lock]] = locks[lock]
	sd[locks[lock]] = std_dev