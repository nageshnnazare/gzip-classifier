/**
 * @file parser.cpp
 * @brief Implementation of the zero-copy CSV parser used by @ref Knn.
 *
 * Parsing algorithm
 * -----------------
 * A single left-to-right scan over @c _fileBuffer tracks:
 * - @c inQuotes — whether the cursor is inside a double-quoted field.
 * - @c cellStart — beginning of the current cell in the buffer.
 *
 * Delimiters:
 * - @c ',' outside quotes ends the current cell.
 * - @c '\n' outside quotes ends the current cell and pushes a completed row.
 *
 * Limitations (by design for current datasets):
 * - Does not unescape doubled quotes (@c "" -> @c ") inside quoted fields.
 * - Treats @c '\r' as cell content; files with CRLF line endings retain a
 *   trailing @c '\r' on the last column of each row.
 * - Does not trim whitespace around cells.
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>

#include <parser.hpp>

CsvParser::CsvParser(const std::string &filename) : _filename{filename} {}

bool CsvParser::parseData() {
  // Open at end first so tellg() returns total byte length in one syscall.
  std::ifstream file{_filename, std::ios::binary | std::ios::ate};
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << _filename << std::endl;
    return false;
  }

  // Slurp the entire file; string_views below point into this buffer.
  std::streamsize size{file.tellg()};
  file.seekg(0, std::ios::beg);
  _fileBuffer.resize(size);
  if (!file.read(_fileBuffer.data(), size)) {
    return false;
  }
  file.close();

  // Heuristic reserve: ~100 bytes per row for typical news CSV rows.
  _data.reserve(size / 100);

  const char *start{_fileBuffer.data()};
  const char *end{start + size};
  const char *ptr{start};

  CsvRow currentRow{};
  currentRow.reserve(10); // AG News / Yahoo Answers have ≤ 4 columns.

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

  // Files without a trailing newline still have one final row to flush.
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

CsvParser::OwnedData CsvParser::takeParsedData() {
  OwnedData owned;
  owned.fileBuffer = std::move(_fileBuffer);
  owned.rows = std::move(_data);
  return owned;
}
