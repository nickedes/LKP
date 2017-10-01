from json import load
from subprocess import call
import matplotlib.pyplot as plt


def plot(l, mean, sd):
	# sample
	print(l, mean, sd)
	leg = ["SPINLOCK", "RWLOCK", "SEQLOCK", "RCU", "RWLOCK_CUSTOM"]
	plt.xticks(l, leg)
	plt.errorbar(l, mean, sd, linestyle='None', marker='o')
	plt.legend()
	plt.show()
	plt.savefig("stats.png")   # save the figure to file
	plt.close()

locks = {"SPINLOCK": 1, "RWLOCK": 2,
		 "SEQLOCK": 3, "RCU": 4, "RWLOCK_CUSTOM": 5}

numthreads = 10
ops_per_thread = 5000000

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

# Get saved results

with open('data.json', 'r') as fp:
	results = load(fp)

mean = [0]*(len(locks)+1)
sd = [0]*(len(locks)+1)
l = [0]*(len(locks)+1)

for lock in results:
	# observations for a lock and different percentage of read ops
	percent_readops = sorted(results[lock])
	time = [results[lock][readop] for readop in percent_readops]
	plt.plot(percent_readops, time, marker="o")
	plt.xlabel('Percentage of read operations --->')
	plt.ylabel('Time taken (in secs) --->')
	plt.title("Plot for " + lock)
	plt.show()
	plt.savefig(lock + "-graph.png")   # save the figure to file
	plt.close()


for lock in results:
	# calculate total time taken for each lock
	totalTime = sum(results[lock].values())
	# Avg time
	avgTime = totalTime/len(results[lock].values())
	variance = 0
	# calculate Variance
	for val in results[lock].values():
		variance += (avgTime - val)**2

	variance = variance/len(results[lock].values())
	std_dev = variance**0.5
	# print(lock, ", Total time - ", totalTime, "Avg Time - ", avgTime,
	#       "Variance - ", variance, "Standard Deviation - ", std_dev)

	# Save avg and standard deviation observed
	mean[locks[lock]] = avgTime
	l[locks[lock]] = locks[lock]
	sd[locks[lock]] = std_dev

plot(l[1:], mean[1:], sd[1:])
