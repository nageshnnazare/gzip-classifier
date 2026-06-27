/**
 * @file knn.cpp
 * @brief k-nearest-neighbors classifier built on gzip-normalized compression
 * distance.
 *
 * Implementation notes
 * --------------------
 * - Parsed CSV rows are moved out of a temporary @ref CsvParser via
 *   @ref CsvParser::takeParsedData so @c _data and @c _fileBuffer are owned
 *   here.  Cell @c string_view values point into @c _fileBuffer.
 * - Label parsing strips optional surrounding double quotes (Yahoo Answers
 *   wraps numeric ids) and skips rows whose first column is not numeric
 *   (e.g. the AG News header @c "Class Index").
 */

#include <cctype>
#include <iostream>
#include <unordered_map>

#include <knn.hpp>

Knn::Knn(const std::string &train, bool debug) : _train{train}, _debug{debug} {
  CsvParser csv{train};
  csv.parseData();
  if (_debug) {
    csv.printData(10);
  }

  CsvParser::OwnedData parsed = csv.takeParsedData();
  _fileBuffer = std::move(parsed.fileBuffer);
  _data = std::move(parsed.rows);
}

void Knn::analyzeDataTest() {
  std::unordered_map<int, int> classes{};

  for (const auto &row : _data) {
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

void Knn::compressDecompressDataTest() {
  constexpr int kMaxRows = 10;
  int count = 0;

  std::cout << " ======== Compress / Decompress ======= \n";
  for (const auto &row : _data) {
    if (count >= kMaxRows || row.size() < 2) {
      continue;
    }

    const std::string text{row.at(1)};
    const auto compressed = _compressor.compressString(text);
    const std::string restored = _compressor.decompressString(compressed);

    std::cout << " Row " << count << " original=" << text.size()
              << " compressed=" << compressed.size() << " ok=" << std::boolalpha
              << (restored == text) << "\n";
    count++;
  }
  std::cout << " ====================================== \n";
}
