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
#include <optional>
#include <string>
#include <unordered_map>

#include <knn.hpp>

namespace {

/** @return Parsed class id, or @c std::nullopt for headers / bad labels. */
std::optional<int> parseClassLabel(std::string_view label) {
  if (label.empty()) {
    return std::nullopt;
  }
  if (label.front() == '"') {
    label.remove_prefix(1);
  }
  if (!label.empty() && label.back() == '"') {
    label.remove_suffix(1);
  }
  if (label.empty() ||
      !std::isdigit(static_cast<unsigned char>(label.front()))) {
    return std::nullopt;
  }
  return std::stoi(std::string{label});
}

/** @return @c true when @p label is a numeric class id (skips CSV headers). */
bool isNumericLabel(std::string_view label) {
  return parseClassLabel(label).has_value();
}

/**
 * @brief Join non-label text columns into one document string.
 *
 * NCD is meaningful on full documents; compressing a single short field
 * (or the header row) will almost always produce a *larger* gzip payload
 * because of format overhead.
 */
std::string joinRowText(const CsvParser::CsvRow &row) {
  std::string text;
  for (std::size_t col = 1; col < row.size(); ++col) {
    if (col > 1) {
      text.push_back(' ');
    }
    text.append(row[col]);
  }
  return text;
}

} // namespace

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
    if (row.empty() || !isNumericLabel(row.at(0))) {
      continue;
    }

    const int classIndex = *parseClassLabel(row.at(0));
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
    if (count >= kMaxRows || row.size() < 2 || !isNumericLabel(row.at(0))) {
      continue;
    }

    const std::string text = joinRowText(row);
    const auto compressed = _compressor.compressString(text);
    const std::string restored = _compressor.decompressString(compressed);

    const double ratio =
        text.empty() ? 0.0
                     : static_cast<double>(text.size() - compressed.size()) /
                           text.size();

    std::cout << " Row " << count << " original=" << text.size()
              << " compressed=" << compressed.size()
              << " saving=" << ratio * 100 << "% ok=" << std::boolalpha
              << (restored == text) << "\n";
    count++;
  }
  std::cout << " ====================================== \n";
}
