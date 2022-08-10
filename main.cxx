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

// Prime numbers just below 512, 1024, 2048, 4096.
const vector<int> PRIMES = {509, 1021, 2039, 4093};

template <class G, class V, class FH>
void runLouvainWith(const G& x, V M, int repeat, const char *name, FH fh) {
  using K = typename G::key_type;
  // Using limited-capacity accumulator hashtable (true).
  for (size_t accumulatorCapacity : PRIMES) {
    LouvainResult<K> a = louvainSeq<true>(fh, x, {repeat, accumulatorCapacity});
    auto fc = [&](auto u) { return a.membership[u]; };
    auto Q  = modularity(x, fc, M, 1.0f);
    printf("[%09.3f ms; %04d iterations; %03d passes; %01.6f modularity] %s {acc_capacity=%zu}\n", a.time, a.iterations, a.passes, Q, name, accumulatorCapacity);
  }
}


template <class G>
void runLouvain(const G& x, int repeat) {
  using K = typename G::key_type;
  auto M = edgeWeight(x)/2;
  auto Q = modularity(x, M, 1.0f);
  printf("[%01.6f modularity] noop\n", Q);

  // Using full-capacity accumulator hashtable (false).
  do {
    auto fh = [](auto k, auto C) { return k; };
    LouvainResult<K> a = louvainSeq<false>(fh, x, {repeat});
    auto fc = [&](auto u) { return a.membership[u]; };
    auto Q  = modularity(x, fc, M, 1.0f);
    printf("[%09.3f ms; %04d iterations; %03d passes; %01.6f modularity] louvainSeq\n", a.time, a.iterations, a.passes, Q);
  } while(0);

  runLouvainWith(x, M, repeat, "louvainSeqDivision", [](auto k, auto C) {
    return k % C;
    // - https://en.wikipedia.org/wiki/Hash_table
  });
  runLouvainWith(x, M, repeat, "louvainSeqMultiplication", [](auto k, auto C) {
    return k<C? k : size_t(C*k*1.618033988749894) % C;
    // - https://en.wikipedia.org/wiki/Hash_table
  });
  runLouvainWith(x, M, repeat, "louvainSeqDjb2", [](auto k, auto C) {
    auto l = k & 0xFFFF;
    auto h = k >> 16;
    return k<C? k : (((l<<5) + l) + h) % C;
    // - http://www.cse.yorku.ca/~oz/hash.html
  });
  runLouvainWith(x, M, repeat, "louvainSeqSdbm", [](auto k, auto C) {
    auto l = k & 0xFFFF;
    auto h = k >> 16;
    return k<C? k : (h + (l<<6) + (l<<16) - l) % C;
    // - http://www.cse.yorku.ca/~oz/hash.html
  });
  runLouvainWith(x, M, repeat, "louvainSeqMagic", [](auto k, auto C) {
    if (k<C) return size_t(k);
    k = ((k >> 16) ^ k) * 0x45d9f3b;
    k = ((k >> 16) ^ k) * 0x45d9f3b;
    k = (k >> 16) ^ k;
    return k % C;
    // - https://stackoverflow.com/a/12996028/1413259
  });
}


int main(int argc, char **argv) {
  using K = int;
  using V = float;
  char *file = argv[1];
  int repeat = argc>2? stoi(argv[2]) : 5;
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
