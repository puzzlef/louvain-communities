#include <utility>
#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include "src/main.hxx"

using namespace std;




// The problem with using a large accumulator hashtable is that it is not
// feasible on a GPU, where we have a large number of threads, but a small
// amount of working memory (shared memory). To alleviate this, we may use
// a hash table instead. To guarentee that we have the space to store linked
// communities (in the worst case, where most neighbors are in unique
// communities), we must reserve enough space for all the community ids and
// linked community weights, asusming they are all unique. For high-degree
// vertices, this would have to be done in the global memory instead (which
// is slow). In addition, we would have to use atomicCAS() operations in
// order to avoid collisions which can further drop performance. Therefore,
// here my idea is to look if we can simply do away with large hash tables
// and collision resolution altogether, by simply considering identical hash
// keys as identical communities. This may lead to bad communities, but that
// is what this experiment is for. If they do yield good communities it can
// be a big win in performance. Note that such a scheme is only likely to
// cause issues only in the first few iterations, when we have a large number
// of communities. In addition, as there is potential for multiple community
// ids to be combined, we will consider the new community id to be the hash
// key. We can adjust the hash function or the capacity of the hash table to
// observe the impact on modularity, which could an indicator of the amount
// of (ignored) community id collisions that might have occured behind the
// scenes.

// Prime numbers just below 2, 3, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096.
const vector<int> PRIMES = {2, 3, 7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093};


template <class G>
void runLouvain(const G& x, int repeat) {
  using K = typename G::key_type;
  using V = typename G::edge_value_type;
  auto M = edgeWeight(x)/2;
  auto Q = modularity(x, M, 1.0f);
  printf("[%01.6f modularity] noop\n", Q);

  // Using full-capacity accumulator hashtable, no iteration pre-processing.
  do {
    LouvainResult<K> a = louvainSeq<false>(x, {repeat});
    auto fc = [&](auto u) { return a.membership[u]; };
    auto Q  = modularity(x, fc, M, 1.0f);
    printf("[%09.3f ms; %04d iterations; %03d passes; %01.6f modularity] louvainSeq\n", a.time, a.iterations, a.passes, Q);
  } while(0);

  // Using full-capacity accumulator hashtable, but with iteration pre-processing.
  for (V subsetPercent=0.05; subsetPercent<=1; subsetPercent+=0.05) {
    for (int maxSubIterations=1; maxSubIterations<=256; maxSubIterations*=2) {
      LouvainResult<K> a = louvainSeq<false>(x, {repeat, subsetPercent, maxSubIterations});
      auto fc = [&](auto u) { return a.membership[u]; };
      auto Q  = modularity(x, fc, M, 1.0f);
      printf("[%09.3f ms; %04d iterations; %03d passes; %01.6f modularity] louvainSeq {sub_percent=%1.2f, max_sub_iterations=%d}\n", a.time, a.iterations, a.passes, Q, subsetPercent, maxSubIterations);
    }
  }

  // Using limited-capacity accumulator hashtable, and with iteration pre-processing.
  size_t accumulatorCapacity = 4093;
  for (V subsetPercent=0.05; subsetPercent<=1; subsetPercent+=0.05) {
    for (int maxSubIterations=1; maxSubIterations<=256; maxSubIterations*=2) {
      LouvainResult<K> a = louvainSeq<true>(x, {repeat, subsetPercent, maxSubIterations, accumulatorCapacity});
      auto fc = [&](auto u) { return a.membership[u]; };
      auto Q  = modularity(x, fc, M, 1.0f);
      printf("[%09.3f ms; %04d iterations; %03d passes; %01.6f modularity] louvainSeq {sub_percent=%1.2f, max_sub_iterations=%d, acc_capacity=%zu}\n", a.time, a.iterations, a.passes, Q, subsetPercent, maxSubIterations, accumulatorCapacity);
    }
  }
}


int main(int argc, char **argv) {
  using K = int;
  using V = float;
  char *file = argv[1];
  int repeat = argc>2? stoi(argv[2]) : 1;
  OutDiGraph<K, None, V> x; V w = 1;
  printf("Loading graph %s ...\n", file);
  readMtxW(x, file); println(x);
  auto y  = symmetricize(x); print(y); printf(" (symmetricize)\n");
  auto fl = [](auto u) { return true; };
  // selfLoopU(y, w, fl); print(y); printf(" (selfLoopAllVertices)\n");
  runLouvain(y, repeat);
  printf("\n");
  return 0;
}
