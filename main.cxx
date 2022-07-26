#include <utility>
#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include "src/main.hxx"

using namespace std;




template <class G>
void runLouvain(const G& x, int repeat) {
  using K = typename G::key_type;
  using V = typename G::edge_value_type;
  V resolution = V(1);
  V passTolerance = V(0);
  auto M = edgeWeight(x)/2;
  auto Q = modularity(x, M, 1.0f);
  printf("[%01.6f modularity] noop\n", Q);

  // Run louvain algorithm.
  for (V toleranceDeclineFactor=V(10); toleranceDeclineFactor<=V(1e+4); toleranceDeclineFactor*=V(10)) {
    for (V tolerance=V(1e-0); tolerance>=V(1e-12); tolerance*=V(0.1)) {
      LouvainResult<K> a = louvainSeq(x, {repeat, resolution, tolerance, passTolerance, toleranceDeclineFactor});
      auto fc = [&](auto u) { return a.membership[u]; };
      auto Q  = modularity(x, fc, M, 1.0f);
      printf("[%09.3f ms; %04d iters.; %03d passes; %01.9f modularity] louvainSeq {tolerance: %1.1e, tol_dec_factor: %1.1e}\n", a.time, a.iterations, a.passes, Q, tolerance, toleranceDeclineFactor);
    }
  }
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
