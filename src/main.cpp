/**
 * @file main.cpp
 * @brief Entry point for the gzip-distance text classifier prototype.
 *
 * Current behaviour
 * -----------------
 * 1. Smoke-test @ref Compressor with a round-trip compress/decompress and
 *    print compressed byte lengths (building blocks for NCD).
 * 2. Load an AG News CSV through @ref Knn and print per-class row counts.
 *
 * Future work: accept CLI flags (@c --train, @c --test, @c --k), run k-NN
 * classification on a held-out test set, and report accuracy.
 */

#include <cassert>
#include <iostream>
#include <string>

#include <compressor.hpp>
#include <knn.hpp>

int main() {
  Knn knn{"datasets/ag_news/test.csv", true};
  knn.analyzeDataTest();
  knn.compressDecompressDataTest();

  return 0;
}
