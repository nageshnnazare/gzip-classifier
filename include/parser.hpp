#ifndef PARSER_HPP
#define PARSER_HPP

/**
 * @file parser.hpp
 * @brief Zero-copy CSV reader for labeled text classification datasets.
 *
 * Parses entire files into memory and exposes rows as string_view slices
 * into an internal buffer, avoiding per-cell string allocations.
 */

#include <string>
#include <string_view>
#include <vector>

/**
 * @class CsvParser
 * @brief Streaming-free CSV parser optimized for large training files.
 *
 * Supports quoted fields containing commas and newlines. Parsed values
 * remain valid for the lifetime of the CsvParser instance.
 */
class CsvParser {
public:
  using CsvCell = std::string_view;
  using CsvRow = std::vector<CsvCell>;
  using CsvData = std::vector<CsvRow>;

  /** @brief Bind parser to @p filename; does not read until parseData(). */
  explicit CsvParser(const std::string &filename);
  ~CsvParser() = default;

  /** @brief Read and parse the CSV file into @c _data. @return false on I/O
   * error. */
  bool parseData();

  /** @brief Print every parsed row to stdout. */
  void printData();

  /**
   * @brief Print up to @p numRows sample rows to stdout.
   * @param numRows Maximum number of data rows to display.
   */
  void printData(int numRows);

  /** @return Reference to parsed rows; valid until this object is destroyed. */
  const CsvData &getData() const;

private:
  std::string _filename;
  std::string _fileBuffer;
  CsvData _data;
};

#endif // PARSER_HPP
