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

#include <algorithm>
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

Knn::Knn(const std::string &train, const std::string &test,
         const unsigned int k, bool debug)
    : _train{train}, _test{test}, _k{k}, _debug{debug} {
  {
    CsvParser trainCsv{train};
    trainCsv.parseData();
    if (_debug) {
      std::cout << " === Training Set === \n";
      trainCsv.printData(10);
    }

    CsvParser::OwnedData parsed = trainCsv.takeParsedData();
    _trainBuffer = std::move(parsed.fileBuffer);
    _trainData = std::move(parsed.rows);
  }
  {
    CsvParser testCsv{test};
    testCsv.parseData();
    if (_debug) {
      std::cout << " === Testing Set === \n";
      testCsv.printData(10);
    }

    CsvParser::OwnedData parsed = testCsv.takeParsedData();
    _testBuffer = std::move(parsed.fileBuffer);
    _testData = std::move(parsed.rows);
  }
}

void Knn::analyzeDataTest() {
  {
    std::unordered_map<int, int> classes{};

    for (const auto &row : _trainData) {
      if (row.empty() || !isNumericLabel(row.at(0))) {
        continue;
      }

      const int classIndex = *parseClassLabel(row.at(0));
      classes[classIndex]++;
    }

    std::cout << " === Training data Analysis === \n";
    for (const auto &[key, value] : classes) {
      std::cout << " Class " << key << " : # " << value << "\n";
    }
    std::cout << " ============================== \n";
  }
  {
    std::unordered_map<int, int> classes{};

    for (const auto &row : _testData) {
      if (row.empty() || !isNumericLabel(row.at(0))) {
        continue;
      }

      const int classIndex = *parseClassLabel(row.at(0));
      classes[classIndex]++;
    }

    std::cout << " === Testing data Analysis === \n";
    for (const auto &[key, value] : classes) {
      std::cout << " Class " << key << " : # " << value << "\n";
    }
    std::cout << " ============================== \n";
  }
}

void Knn::compressDecompressDataTest() {
  constexpr int kMaxRows = 1'0;
  int count = 0;

  std::cout << " ======== Compress / Decompress ======= \n";

  for (const auto &row : _trainData) {
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

void Knn::check() {
  analyzeDataTest();
  compressDecompressDataTest();
}

float Knn::calculateNCD(std::string_view textA, std::string_view textB) {
  std::string textAB{};
  textAB.reserve(textA.size() + textB.size());
  textAB.append(textA).append(textB);

  float dA = _compressor.compressString(textA).size();
  float dB = _compressor.compressString(textB).size();
  float dAB = _compressor.compressString(textAB).size();

  float dMin = std::min(dA, dB);
  float dMax = std::max(dA, dB);

  return (dAB - dMin) / (dMax);
}

unsigned int Knn::classify(const std::string &input) {
  std::vector<NCD> ncds{};
  unsigned int iterCount{};

  for (const auto &sample : _trainData) {
    std::cout << "\r Classifying: " << (++iterCount) * 100 / (_trainData.size())
              << "%";

    auto label = parseClassLabel(sample.at(0));

    if (!label.has_value())
      continue;

    std::string train{};
    train.append(sample.at(1)).append(sample.at(2));
    float ncd = calculateNCD(train, input);

    ncds.push_back({ncd, *label});
  }
  std::cout << "\n";

  std::sort(ncds.begin(), ncds.end(),
            [](const NCD &a, const NCD &b) { return a.first > b.first; });

  iterCount = 0;
  for (const auto &ncd : ncds) {
    if (iterCount >= _k)
      break;
    std::cout << " NCD:" << ncd.first << " Label:" << ncd.second << "\n";
    ++iterCount;
  }
  std::cout << " Size:" << ncds.size() << "\n";

  std::cout << "====================\n";

  std::unordered_map<unsigned int, unsigned int> classFreq{};

  iterCount = 0UL;
  for (const auto &ncd : ncds) {
    if (iterCount >= _k)
      break;
    ++classFreq[ncd.second];
    ++iterCount;
  }

  for (const auto &[label, freq] : classFreq) {
    std::cout << " Label:" << label << " Freq:" << freq << "\n";
  }

  std::cout << "====================\n";

  auto maxFreq = std::max_element(
      classFreq.begin(), classFreq.end(),
      [](const auto &a, const auto &b) { return a.second < b.second; });

  return maxFreq->first;
}