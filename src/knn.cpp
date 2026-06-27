/**
 * @file knn.cpp
 * @brief k-nearest-neighbors classifier built on gzip-normalized compression
 * distance.
 *
 * Loads labeled training CSV data via CsvParser and exposes analysis helpers
 * used while the full distance-based classifier is under development.
 */

#include <cctype>
#include <iostream>
#include <unordered_map>

#include <knn.hpp>

Knn::Knn(const std::string &train, bool debug)
    : _train{train}, _csv{train}, _debug{debug} {
  _csv.parseData();
  _data = &_csv.getData();
  if (_debug) {
    _csv.printData(10);
  }
}

void Knn::analyzeData() {
  std::unordered_map<int, int> classes{};
  for (const auto &row : *_data) {
    if (row.empty()) {
      continue;
    }

    std::string label{row.at(0)};
    if (!label.empty() && label.front() == '"') {
      label.erase(0, 1);
    }
    if (!label.empty() && label.back() == '"') {
      label.pop_back();
    }
    if (label.empty() ||
        !std::isdigit(static_cast<unsigned char>(label.front()))) {
      continue;
    }

    const int classIndex = std::stoi(label);
    classes[classIndex]++;
  }

  std::cout << " === Analysis === \n";
  for (const auto &[key, value] : classes) {
    std::cout << " Class " << key << " : # " << value << "\n";
  }
  std::cout << " ================ \n";
}
