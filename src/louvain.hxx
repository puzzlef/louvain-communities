#pragma once
#include <vector>
#include <utility>
#include "_main.hxx"

using std::vector;
using std::move;




template <class T>
struct LouvainOptions {
  int    repeat;
  size_t accumulatorCapacity;
  T   resolution;
  T   tolerance;
  T   passTolerance;
  int maxIterations;
  int maxPasses;

  LouvainOptions(int repeat=1, size_t accumulatorCapacity=0, T resolution=1, T tolerance=0, T passTolerance=0, int maxIterations=500, int maxPasses=500) :
  repeat(repeat), accumulatorCapacity(accumulatorCapacity), resolution(resolution), tolerance(tolerance), passTolerance(passTolerance), maxIterations(maxIterations), maxPasses(maxPasses) {}
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
