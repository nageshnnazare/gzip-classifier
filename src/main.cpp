/**
 * @file main.cpp
 * @brief Entry point for the gzip-distance text classifier prototype.
 *
 * Wires together dataset loading and exploratory analysis. Classification
 * via normalized compression distance will be invoked from here once
 * the Knn distance routines are implemented.
 */

#include <knn.hpp>

int main() {
  Knn knn{"datasets/ag_news/test.csv", true};
  knn.analyzeData();

  return 0;
}
