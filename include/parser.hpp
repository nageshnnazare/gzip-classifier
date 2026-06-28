#ifndef PARSER_HPP
#define PARSER_HPP

/**
 * @file parser.hpp
 * @brief Zero-copy CSV reader for labeled text classification datasets.
 *
 * Design goals
 * ------------
 * - **Single read**: load the entire file into @c _fileBuffer once.
 * - **Zero-copy cells**: each field is a @c std::string_view pointing into
 *   that buffer, so parsing 120k+ rows does not allocate per cell.
 * - **Quoted fields**: commas and newlines inside double quotes are kept as
 *   part of the cell value (required for news/article datasets).
 *
 * Trade-offs
 * ----------
 * - The full file must fit in memory.
 * - Parsed @c string_view values are invalidated if the @ref CsvParser object
 *   is moved or destroyed; callers should consume data while the parser lives.
 * - Quote characters themselves are retained in cell views; downstream code
 *   may strip them when interpreting labels.
 *
 * Expected layout (AG News, Yahoo Answers, etc.)
 * ------------------------------------------------
 * | Column | Role                              |
 * |--------|-----------------------------------|
 * | 0      | Integer class id (may be quoted)  |
 * | 1…n    | One or more free-text fields      |
 */

#include <string>
#include <string_view>
#include <vector>

/**
 * @class CsvParser
 * @brief In-memory CSV parser that exposes rows as @c string_view slices.
 *
 * Typical usage:
 * @code
 *   CsvParser parser{"datasets/ag_news/train.csv"};
 *   if (!parser.parseData()) { ... handle error ... }
 *   for (const auto& row : parser.getData()) { ... }
 * @endcode
 */
class CsvParser {
public:
  using CsvCell = std::string_view;    ///< Single field within a row.
  using CsvRow = std::vector<CsvCell>; ///< All fields on one CSV line.
  using CsvData = std::vector<CsvRow>; ///< Full parsed dataset.

  /**
   * @brief Owns parsed CSV rows and the file buffer that backs their views.
   *
   * @c rows[i][j] are @c string_view slices into @c fileBuffer.  Both members
   * must be moved together from @ref CsvParser so the views stay valid.
   */
  struct OwnedData {
    std::string fileBuffer;
    CsvData rows;
  };

  /**
   * @brief Bind parser to @p filename without reading yet.
   * @param filename Path to a CSV file on disk.
   */
  explicit CsvParser(const std::string &filename);

  /**
   * @brief Read @c _filename and populate @c _data.
   *
   * On success, every row (including an optional header row) is available via
   * @ref getData.  On failure, @c _data may be partial or empty.
   *
   * @return @c false if the file cannot be opened or fully read.
   */
  bool parseData();

  /** @brief Print every parsed row to stdout, cells separated by @c '|'. */
  void printData();

  /**
   * @brief Print up to @p numRows sample rows to stdout.
   * @param numRows Maximum number of rows to display before truncating.
   */
  void printData(int numRows);

  /**
   * @return Const reference to parsed rows.
   * @note Views remain valid only for the lifetime of this @ref CsvParser.
   */
  const CsvData &getData() const;

  /**
   * @brief Move parsed rows and their backing buffer out of this parser.
   *
   * After the call, this parser holds no data (@c _data and @c _fileBuffer are
   * empty).  The returned @ref OwnedData can be stored independently while
   * keeping all @c string_view cells valid.
   */
  OwnedData takeParsedData();

private:
  std::string _filename;   ///< Source path passed to the constructor.
  std::string _fileBuffer; ///< Entire file contents; backing store for views.
  CsvData _data;           ///< Parsed rows pointing into @c _fileBuffer.
};

#endif // PARSER_HPP
