#include <utility>
#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include "src/main.hxx"

using namespace std;




// You can define datatype with -DTYPE=...
#ifndef TYPE
#define TYPE float
#endif




template <class G, class K, class V>
double getModularity(const G& x, const LouvainResult<K>& a, V M) {
  auto fc = [&](auto u) { return a.membership[u]; };
  return modularity(x, fc, M, V(1));
}


template <class G>
void runLouvain(const G& x, int repeat) {
  using K = typename G::key_type;
  using V = typename G::edge_value_type;
  vector<K> *init = nullptr;
  V resolution    = V(1);
  V passTolerance = V(0);
  V toleranceDeclineFactor = V(10);
  auto M = edgeWeight(x)/2;
  auto Q = modularity(x, M, 1.0f);
  printf("[%01.6f modularity] noop\n", Q);

  for (V tolerance=V(1e-0); tolerance>=V(1e-6); tolerance*=V(0.1)) {
    LouvainOptions<V> o = {repeat, resolution, tolerance, passTolerance, toleranceDeclineFactor};
    // Run louvain algorithm with l1-norm.
    LouvainResult<K> a1 = louvainSeq<1>(x, init, o);
    printf("[%09.3f ms; %04d iters.; %03d passes; %01.9f modularity] louvainSeqL1Norm {tolerance: %1.1e}\n", a1.time, a1.iterations, a1.passes, getModularity(x, a1, M), tolerance);
    // Run louvain algorithm with l2-norm.
    LouvainResult<K> a2 = louvainSeq<2>(x, init, o);
    printf("[%09.3f ms; %04d iters.; %03d passes; %01.9f modularity] louvainSeqL2Norm {tolerance: %1.1e}\n", a2.time, a2.iterations, a2.passes, getModularity(x, a2, M), tolerance);
    // Run louvain algorithm with li-norm.
    LouvainResult<K> a3 = louvainSeq<3>(x, init, o);
    printf("[%09.3f ms; %04d iters.; %03d passes; %01.9f modularity] louvainSeqLiNorm {tolerance: %1.1e}\n", a3.time, a3.iterations, a3.passes, getModularity(x, a3, M), tolerance);
  }
}


int main(int argc, char **argv) {
  using K = int;
  using V = TYPE;
  char *file = argv[1];
  int repeat = argc>2? stoi(argv[2]) : 5;
  OutDiGraph<K, None, V> x; V w = 1;
  printf("Loading graph %s ...\n", file);
  readMtxW(x, file); println(x);
  auto y = symmetricize(x); print(y); printf(" (symmetricize)\n");
  // auto fl = [](auto u) { return true; };
  // selfLoopU(y, w, fl); print(y); printf(" (selfLoopAllVertices)\n");
  runLouvain(y, repeat);
  printf("\n");
  return 0;
}
