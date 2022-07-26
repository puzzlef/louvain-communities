#pragma once
#include <vector>
#include <utility>
#include "_main.hxx"

using std::vector;
using std::move;




template <class T>
struct LouvainOptions {
  int repeat;
  T   resolution;
  T   tolerance;
  T   passTolerance;
  T   tolerenceDeclineFactor;
  int maxIterations;
  int maxPasses;

  LouvainOptions(int repeat=1, T resolution=1, T tolerance=0, T passTolerance=0, T tolerenceDeclineFactor=1, int maxIterations=500, int maxPasses=500) :
  repeat(repeat), resolution(resolution), tolerance(tolerance), passTolerance(passTolerance), tolerenceDeclineFactor(tolerenceDeclineFactor), maxIterations(maxIterations), maxPasses(maxPasses) {}
};


template <class K>
struct LouvainResult {
  vector<K> membership;
  int   iterations;
  int   passes;
  float time;

  LouvainResult(vector<K>&& membership, int iterations=0, int passes=0, float time=0) :
  membership(membership), iterations(iterations), passes(passes), time(time) {}

  LouvainResult(vector<K>& membership, int iterations=0, int passes=0, float time=0) :
  membership(move(membership)), iterations(iterations), passes(passes), time(time) {}
};
