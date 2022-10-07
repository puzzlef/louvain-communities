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
  V resolution = V(1);
  V tolerance  = V(1e-2);
  V passTolerance = V(0);
  V toleranceDeclineFactor = V(10);
  auto M = edgeWeight(x)/2;
  auto Q = modularity(x, M, 1.0f);
  printf("[%01.6f modularity] noop\n", Q);
  LouvainOptions<V> o = {repeat, resolution, tolerance, passTolerance, toleranceDeclineFactor};

  // Run louvain algorithm (standard).
  LouvainResult<K> al = louvainSeqStatic(x, init, o);
  printf("[%09.3f ms; %04d iters.; %03d passes; %01.9f modularity] louvainSeq\n", al.time, al.iterations, al.passes, getModularity(x, al, M));
  // Run louvain algorithm (simple first move).
  LouvainResult<K> bl = louvainSeqStatic<true>(x, init, o);
  printf("[%09.3f ms; %04d iters.; %03d passes; %01.9f modularity] louvainSeqFirst\n", bl.time, bl.iterations, bl.passes, getModularity(x, bl, M));
}


int main(int argc, char **argv) {
  using K = int;
  using V = float;
  char *file = argv[1];
  int repeat = argc>2? stoi(argv[2]) : 5;
  OutDiGraph<K, None, V> x;  // V w = 1;
  printf("Loading graph %s ...\n", file);
  readMtxW(x, file); println(x);
  auto y  = symmetricize(x); print(y); printf(" (symmetricize)\n");
  // auto fl = [](auto u) { return true; };
  // selfLoopU(y, w, fl); print(y); printf(" (selfLoopAllVertices)\n");
  runLouvain(y, repeat);
  printf("\n");
  return 0;
}
