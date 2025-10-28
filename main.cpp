#include <iostream>
#include <random>
#include <thread>

#include "DNSCache.h"

using namespace lru;

const int numThreads = 8;
const int cacheSize1 = 10;
const int demandSize = 1000;
const int durationLimit = 0;

DNSCache& aecm = DNSCache::Instance(cacheSize1);
std::vector<std::string> strings;
std::atomic<long> globalHitCount;
std::atomic<long> globalTotalCount;
volatile bool stop = false;

void threadMain();

int main(int argc, char** argv) {
  using std::chrono::duration;
  using std::chrono::duration_cast;
  using std::chrono::seconds;
  using std::chrono::steady_clock;

  for (int i = 0; i < demandSize; i++) {
    std::string s = std::string(100, 'x') + std::to_string(i);
    strings.push_back(std::move(s));
  }

  std::vector<std::thread> threads;
  threads.reserve(numThreads);
  auto startTime = steady_clock::now();
  auto oneSecond = seconds(1);

  for (int i = 0; i < numThreads; i++) {
    threads.push_back(std::thread(threadMain));
  }
  if (durationLimit == 0) {
    long prevTotalCount = 0, prevHitCount = 0;
    for (;;) {
      std::this_thread::sleep_for(oneSecond);
      long totalCount = globalTotalCount.load();
      long hitCount = globalHitCount.load();
      printf("rate = %.5g kreq/s, hit ratio = %.3g%%\n",
             (totalCount - prevTotalCount) / 1000.,
             (double)(hitCount - prevHitCount) / (totalCount - prevTotalCount) *
                 100);

      prevTotalCount = totalCount;
      prevHitCount = hitCount;
    }
  } else {
    std::this_thread::sleep_for(oneSecond * durationLimit);
    long totalCount = globalTotalCount.load();
    long hitCount = globalHitCount.load();
    auto totalTime =
        duration_cast<duration<double>>(steady_clock::now() - startTime);
    stop = true;
    for (int i = 0; i < numThreads; i++) {
      threads[i].join();
    }
    printf("type\tthreads\tcache\tdemand\tduration\trate\tratio\n");
    printf("%s\t%d\t%d\t%d\t%g\t%g\t%g%%\n", "lru", numThreads, cacheSize1,
           demandSize, totalTime.count(),
           totalCount / 1000. / totalTime.count(),
           (double)hitCount / totalCount * 100);
  }
  return 0;
}

void threadMain() {
  int cacheSize = strings.size();

  std::mt19937 generator(std::hash<pthread_t>()(pthread_self()));
  std::uniform_int_distribution<int> rand(0, cacheSize - 1);

  while (!stop) {
    long hitCount = 0;
    long totalCount = 0;
    for (int j = 0; j < 1000; j++) {
      int r = rand(generator);
      if (!aecm.resolve(strings[r]).empty()) {
        hitCount++;
      } else {
        aecm.update(strings[r], std::to_string(j));
      }
      totalCount++;
    }

    globalHitCount += hitCount;
    globalTotalCount += totalCount;
  }
}
