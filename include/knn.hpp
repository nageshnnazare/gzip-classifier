#ifndef KNN_HPP
#define KNN_HPP

/**
 * @file knn.hpp
 * @brief Public interface for the gzip-distance k-nearest-neighbors classifier.
 *
 * This module will eventually orchestrate three concerns:
 *
 *   1. **Data** — labeled text rows loaded through @ref CsvParser.
 *   2. **Distance** — Normalized Compression Distance (NCD) via @ref
 * Compressor.
 *   3. **Voting** — pick the majority label among the @c k training samples
 *      with smallest NCD to a query string.
 *
 * Current scope is limited to dataset loading and exploratory statistics
 * (@ref analyzeData).  Distance computation and classification will be added
 * as the project matures.
 *
 * @see Compressor for the gzip primitives underlying NCD.
 * @see CsvParser for zero-copy CSV ingestion.
 */

#include "compressor.hpp"
#include <string>

#include <parser.hpp>
#include <string_view>

/**
 * @class Knn
 * @brief k-NN text classifier using gzip-based Normalized Compression Distance.
 *
 * Loads a training CSV via @ref CsvParser, then takes ownership of the parsed
 * rows and backing buffer through move semantics.  Column 0 of each row is the
 * integer class label; remaining columns hold text fields for distance work.
 */
class Knn {
public:
  using NCD = std::pair<float, unsigned int>; ///< <distance, class>

  /**
   * @brief Load training data from a CSV file.
   *
   * Parses @p train immediately.  When @p debug is @c true, prints the first
   * ten rows through @ref CsvParser::printData(int) for a quick sanity check.
   *
   * @param train  Filesystem path to a labeled CSV (column 0 = class id).
   * @param debug  When @c true, echo a sample of parsed rows to stdout.
   */
  Knn(const std::string &train, const std::string &test, const unsigned int k,
      bool debug);

  void check();

  unsigned int classify(const std::string &input);

private:
  std::string _train; ///< Copy of the training file path (for logging).
  std::string _test;  ///< Copy of the test file path (for logging).

  std::string _trainBuffer; ///< Backing store for @c string_view cells in @c
                            ///< _trainData.
  std::string
      _testBuffer; ///< Backing store for @c string_view cells in @c _testData.

  CsvParser::CsvData _trainData; ///< Parsed rows moved from @ref CsvParser.
  CsvParser::CsvData _testData;  ///< Parsed rows moved from @ref CsvParser.

  Compressor _compressor{}; ///< Gzip compressor for NCD experiments.

  const unsigned int _k;
  bool _debug; ///< When true, emit verbose parse diagnostics.

private:
  /**
   * @brief Print per-class row counts from the loaded training set.
   *
   * Iterates every parsed row, parses the label in column 0, and accumulates
   * counts in an @c std::unordered_map.  Header rows and non-numeric labels
   * are skipped silently.
   *
   * Useful for validating dataset layout and class balance before running
   * classification experiments.
   */
  void analyzeDataTest();

  void compressDecompressDataTest();

  float calculateNCD(std::string_view textA, std::string_view textB);
};

#endif // KNN_HPP
