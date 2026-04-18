#include "parser.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string_view>

CsvParser::CsvParser(const std::string& filename) : _filename(filename) {}

CsvParser::~CsvParser() {}

bool 
CsvParser::parseData() {
    std::ifstream file(_filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << _filename << std::endl;
        return false;
    }

    // Read entire file into buffer
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    _fileBuffer.resize(size);
    if (!file.read(_fileBuffer.data(), size)) {
        return false;
    }
    file.close();

    // Estimate row count for reservation (using average line length)
    _data.reserve(size / 100); 

    const char* start = _fileBuffer.data();
    const char* end = start + size;
    const char* ptr = start;

    std::vector<std::string_view> currentRow;
    currentRow.reserve(10); // CSV column count

    bool inQuotes = false;
    const char* cellStart = start;

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

    // Handle last row if no trailing newline
    if (cellStart < end) {
        currentRow.emplace_back(cellStart, end - cellStart);
        _data.push_back(std::move(currentRow));
    }

    return true;
}

void CsvParser::printData() {
    for (const auto& row : _data) {
        for (const auto& cell : row) {
            std::cout << cell << "|";
        }
        std::cout << std::endl;
    }
}

void CsvParser::printData(int numRows) {
    int count = 0;
    for (const auto& row : _data) {
        if (count >= numRows) break;
        for (const auto& cell : row) {
            std::cout << cell << "|";
        }
        std::cout << std::endl;
        count++;
    }
}

const std::vector<std::vector<std::string_view>>& CsvParser::getData() const {
    return _data;
}