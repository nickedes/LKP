from json import *
import matplotlib.pyplot as plt
from subprocess import call
from time import time, sleep


def plot(l, mean, sd):
    # plot the mean and deviation for all locks
    leg = ["SPINLOCK", "RWLOCK", "SEQLOCK", "RCU", "RWLOCK_CUSTOM"]
    min_index = mean.index(min(mean))
    print("###########################################################################")
    print("Fastest Lock among all these - " + leg[min_index])
    print("Its average time taken (in secs) is - ", mean[min_index])
    max_index = mean.index(max(mean))
    print("Slowest Lock among all these - " + leg[max_index])
    print("Its average time taken (in secs) is - ", mean[max_index])

    plt.xticks(l, leg)
    plt.ylabel('Average Time taken (in secs) --->')
    plt.xlabel('Locks --->')
    plt.errorbar(l, mean, sd, linestyle='None', marker='o')
    plt.savefig("stats.png")   # save the figure to file
    plt.show()
    plt.close()


def compareLocks(results):
    """
        Compare the time taken by each lock 
    """
    fig, (ax, ax2) = plt.subplots(2, sharex=True)
    # plot the same data on both axes
    ax.plot(list(results["RCU"].keys()), list(
        results["RCU"].values()), marker="o")
    legends = []

    for lock in results:
        if lock != "RCU":
            legends.append(lock)
            ax2.plot(list(results[lock].keys()), list(
                results[lock].values()), marker="o")

    print(legends)
    limit_from, limit_to = min(results["RCU"].values()), max(
        results["RCU"].values())

    # zoom-in / limit the view to different portions of the data
    ax.set_ylim(limit_from, limit_to+500)
    ax2.set_ylim(100, 111)  # most of the data

    # hide the spines between ax and ax2
    # ax.spines['bottom'].set_visible(False)
    ax2.spines['top'].set_visible(False)

    # Make the spacing between the two axes a bit smaller
    plt.subplots_adjust(wspace=0.15)
    plt.xlabel('Percentage of read operations --->')
    plt.ylabel('Time taken (in secs) --->')
    ax.legend(["RCU"], loc='upper right', fontsize='small')
    ax2.legend(legends, loc='upper right', fontsize='small')
    plt.savefig("Locks-Comparison-graph.png")   # save the figure to file
    plt.show()
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

print("###########################################################################")
sleep(1)

for lock in results:
    print("LOCK -   " + lock)
    print("Percentage of Read operations : Time Taken (in seconds)")
    print()
    print(dumps(results[lock], indent=1))
    print()
    sleep(2)


print("###########################################################################")
print("Graphs For each of the locks showing there behaviour for different read ops percentage")
print("NOTE : Close EACH Graph figure to see further")
plt.xlabel('Percentage of read operations --->')
plt.ylabel('Time taken (in secs) --->')
plt.title("Comparison")
sleep(3)

legends = []

for lock in results:
    # observations for a lock and different percentage of read ops
    percent_readops = sorted(results[lock])
    times = [results[lock][readop] for readop in percent_readops]
    plt.plot(percent_readops, times, marker="o")
    legends.append(lock)

plt.legend(legends, loc='upper right', fontsize='small')
plt.savefig("Locks-Comparison-graph.png")   # save the figure to file
plt.show()
plt.close()

compareLocks(results)

for lock in results:
    # observations for a lock and different percentage of read ops
    percent_readops = sorted(results[lock])
    times = [results[lock][readop] for readop in percent_readops]
    plt.plot(percent_readops, times, marker="o")
    plt.xlabel('Percentage of read operations --->')
    plt.ylabel('Time taken (in secs) --->')
    plt.title("Plot for " + lock)
    plt.savefig(lock + "-graph.png")   # save the figure to file
    plt.show()
    plt.close()

print("###########################################################################")

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
    print()
    print("Results for " + lock)
    print("Total time for all readops (in secs) - ", totalTime)
    print("Avg Time (in secs) - ", avgTime)
    print("Variance (in secs^2) - ", variance)
    print("Standard Deviation (in secs) - ", std_dev)
    print()
    print("###########################################################################")
    sleep(3)
    # Save avg and standard deviation observed
    mean[locks[lock]] = avgTime
    l[locks[lock]] = locks[lock]
    sd[locks[lock]] = std_dev

plot(l[1:], mean[1:], sd[1:])
