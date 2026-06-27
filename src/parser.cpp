/**
 * @file parser.cpp
 * @brief Implementation of the zero-copy CSV parser used by Knn.
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include <parser.hpp>

CsvParser::CsvParser(const std::string &filename) : _filename{filename} {}

bool CsvParser::parseData() {
  std::ifstream file{_filename, std::ios::binary | std::ios::ate};
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << _filename << std::endl;
    return false;
  }

  // Read entire file into a single buffer so string_views stay stable.
  std::streamsize size{file.tellg()};
  file.seekg(0, std::ios::beg);
  _fileBuffer.resize(size);
  if (!file.read(_fileBuffer.data(), size)) {
    return false;
  }
  file.close();

  _data.reserve(size / 100);

  const char *start{_fileBuffer.data()};
  const char *end{start + size};
  const char *ptr{start};

  CsvRow currentRow{};
  currentRow.reserve(10);

  bool inQuotes{false};
  const char *cellStart{start};

  while (ptr < end) {
    if (*ptr == '"') {
      inQuotes = !inQuotes;
    } else if (*ptr == ',' && !inQuotes) {
      currentRow.emplace_back(cellStart, ptr - cellStart);
      cellStart = ptr + 1;
    } else if (*ptr == '\n' && !inQuotes) {
      currentRow.emplace_back(cellStart, ptr - cellStart);
      _data.push_back(std::move(currentRow));

      currentRow.clear();
      currentRow.reserve(10);
      cellStart = ptr + 1;
    }
    ptr++;
  }

  // Handle final row when the file has no trailing newline.
  if (cellStart < end) {
    currentRow.emplace_back(cellStart, end - cellStart);
    _data.push_back(std::move(currentRow));
  }

  return true;
}

void CsvParser::printData() {
  std::cout << " === CSV Data === \n";
  for (const auto &row : _data) {
    for (const auto &cell : row) {
      std::cout << cell << "|";
    }
    std::cout << '\n';
  }
  std::cout << " Total: " << _data.size() << " lines\n";
  std::cout << " ================ \n";
}

void CsvParser::printData(int numRows) {
  std::cout << " === CSV Data === \n";
  int count = 0;
  for (const auto &row : _data) {
    if (count >= numRows) {
      break;
    }
    for (const auto &cell : row) {
      std::cout << cell << "|";
    }
    std::cout << '\n';
    count++;
  }
  std::cout << "   ...(" << _data.size() - numRows << " more lines)\n";
  std::cout << " Total: " << _data.size() << " lines\n";
  std::cout << " ================ \n";
}

const CsvParser::CsvData &CsvParser::getData() const { return _data; }
