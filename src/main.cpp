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

namespace {
constexpr const char *agNewsClassNames[] = {"None", "World", "Sports",
                                            "Business", "Sci/Tech"};
const unsigned int k = 4;
} // namespace

int main() {
  bool debug = true;
  Knn knn{"datasets/ag_news/train.csv", "datasets/ag_news/test.csv", k, debug};

  if (debug) {
    knn.check();
  }

  std::string input;
  std::cout << "Input Text: " << "\n> ";
  while (std::getline(std::cin, input)) {
    if (input == "quit" || input == "q" || input == "exit") {
      break;
    }
    unsigned int predictedClass = knn.classify(input);
    predictedClass =
        predictedClass > sizeof(agNewsClassNames) / sizeof(agNewsClassNames[0])
            ? 0
            : predictedClass;

    std::cout << "Predicted Class: (" << predictedClass << ") "
              << agNewsClassNames[predictedClass] << "\n";
    std::cout << "Input Text: " << "\n> ";
  }
  return 0;
}
