#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <vector>
#include <string>
#include <string_view>

class CsvParser {
public:
    CsvParser(const std::string& filename);
    ~CsvParser();
    bool parseData();
    void printData();
    void printData(int numRows);
    const std::vector<std::vector<std::string_view>>& getData() const;

private:
    std::string _filename;
    std::string _fileBuffer;
    std::vector<std::vector<std::string_view>> _data;
};

#endif // PARSER_HPP