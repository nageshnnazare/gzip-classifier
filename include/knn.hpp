#ifndef KNN_HPP
#define KNN_HPP

/**
 * @file knn.hpp
 * @brief Public interface for the gzip-distance k-nearest-neighbors classifier.
 *
 */

#include <string>

#include <parser.hpp>

/**
 * @class Knn
 * @brief k-NN text classifier using gzip compression distance.
 *
 * Owns a CsvParser for the training set and will eventually compute NCD
 * between a query string and stored training rows to vote on the label.
 */
class Knn {

public:
  /**
   * @brief Load training data from a CSV file.
   * @param train Path to the labeled training CSV (column 0 = class id).
   * @param debug When true, print a sample of parsed rows to stdout.
   */
  Knn(const std::string &train, bool debug);
  ~Knn() = default;

  /**
   * @brief Print per-class row counts from the loaded training set.
   *
   * Skips header rows and rows whose first column is not numeric.
   * Useful for validating dataset layout before running classification.
   */
  void analyzeData();

private:
  std::string _train;
  CsvParser _csv;
  bool _debug;
  const CsvParser::CsvData *_data;
};

#endif // KNN_HPP
