#pragma once
#include <utility>
#include <vector>
#include <algorithm>
#include "_main.hxx"
#include "properties.hxx"
#include "duplicate.hxx"
#include "modularity.hxx"
#include "louvain.hxx"

using std::tuple;
using std::vector;
using std::min;




// LOUVAIN-SEQ
// -----------

template <bool FIR=false, class G, class K, class V, class FA, class FP>
auto louvainSeq(const G& x, const vector<K>* q, const LouvainOptions<V>& o, FA fa, FP fp) {
  V   R = o.resolution;
  V   D = o.passTolerance;
  int L = o.maxIterations, l = 0;
  int P = o.maxPasses, p = 0;
  K   S = x.span();
  V   M = edgeWeight(x)/2;
  vector<K> vmov(S), vcom(S), vcs, a(S);
  vector<V> vtot(S), ctot(S), vcout(S);
  float t = measureDurationMarked([&](auto mark) {
    V E  = o.tolerance;
    V Q0 = modularity(x, M, R);
    G y  = duplicate(x);
    fillValueU(vcom, K());
    fillValueU(vtot, V());
    fillValueU(ctot, V());
    mark([&]() {
      louvainVertexWeights(vtot, y);
      louvainInitialize(vcom, ctot, y, vtot);
      if (q) copyValues(*q, vcom, 0, min((*q).size(), vcom.size()));
      if (q) louvainCommunityWeights(ctot, y, vcom, vtot);
      copyValues(vcom, a);
      for (l=0, p=0; p<P;) {
        int m = 0;
        if (p==0) m += louvainMove<FIR>(vmov, vcom, ctot, vcs, vcout, y, vtot, M, R, E, L, fa, fp);
        else      m += louvainMove     (vmov, vcom, ctot, vcs, vcout, y, vtot, M, R, E, L);
        l += m;
        if (m<=1) break;
        // K N0 = y.order();
        y = louvainAggregate(vcs, vcout, y, vcom); ++p;
        // K N1 = y.order();
        // if (N1==N0) break;
        louvainLookupCommunities(a, vcom);
        V Q = D>0? modularity(y, M, R) : Q0 + V(1);
        if (Q-Q0<=D) break;
        fillValueU(vcom, K());
        fillValueU(vtot, V());
        fillValueU(ctot, V());
        louvainVertexWeights(vtot, y);
        louvainInitialize(vcom, ctot, y, vtot);
        E /= o.tolerenceDeclineFactor;
        Q0 = Q;
      }
    });
  }, o.repeat);
  return LouvainResult<K>(a, l, p, t);
}
template <bool FIR=false, class G, class K, class V, class FA>
inline auto louvainSeq(const G& x, const vector<K>* q, const LouvainOptions<V>& o, FA fa) {
  auto fp = [](auto u) {};
  return louvainSeq<FIR>(x, q, o, fa, fp);
}
template <bool FIR=false, class G, class K, class V>
inline auto louvainSeq(const G& x, const vector<K>* q, const LouvainOptions<V>& o) {
  auto fa = [](auto u) { return true; };
  return louvainSeq<FIR>(x, q, o, fa);
}




// LOUVAIN-SEQ-STATIC
// ------------------

template <bool FIR=false, class G, class K, class V=float>
inline auto louvainSeqStatic(const G& x, const vector<K>* q=nullptr, const LouvainOptions<V>& o={}) {
  return louvainSeq<FIR>(x, q, o);
}




// LOUVAIN-SEQ-DYNAMIC-DELTA-SCREENING
// -----------------------------------

template <bool FIR=false, class G, class K, class V>
inline auto louvainSeqDynamicDeltaScreening(const G& x, const vector<tuple<K, K>>& deletions, const vector<tuple<K, K, V>>& insertions, const vector<K>* q, const LouvainOptions<V>& o={}) {
  K S = x.span();
  V R = o.resolution;
  V M = edgeWeight(x)/2;
  const vector<K>& vcom = *q;
  vector<V> vtot(S), ctot(S);
  louvainVertexWeights(vtot, x);
  louvainCommunityWeights(ctot, x, vcom, vtot);
  auto vaff = louvainAffectedVerticesDeltaScreening(x, deletions, insertions, vcom, vtot, ctot, M, R);
  auto fa   = [&](auto u) { return vaff[u]==true; };
  return louvainSeq<FIR>(x, q, o, fa);
}




// LOUVAIN-SEQ-DYNAMIC-FRONTIER
// ----------------------------

template <bool FIR=false, class G, class K, class V>
inline auto louvainSeqDynamicFrontier(const G& x, const vector<tuple<K, K>>& deletions, const vector<tuple<K, K, V>>& insertions, const vector<K>* q, const LouvainOptions<V>& o={}) {
  K S = x.span();
  const vector<K>& vcom = *q;
  auto vaff = louvainAffectedVerticesFrontier(x, deletions, insertions, vcom);
  auto fa = [&](auto u) { return vaff[u]==true; };
  auto fp = [&](auto u) { x.forEachEdgeKey(u, [&](auto v) { vaff[v] = true; }); };
  return louvainSeq<FIR>(x, q, o, fa, fp);
}
