#include "parser.hpp"
#include <iostream>
#include <sstream>

CsvParser::CsvParser(const std::string& filename) : _filename(filename) {}

CsvParser::~CsvParser() {}

bool 
CsvParser::parse() {
    std::ifstream file(_filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << _filename << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            row.push_back(cell);
        }
        _data.push_back(row);
    }
    file.close();
    return true;
}

void CsvParser::print() {
    for (const auto& row : _data) {
        for (const auto& cell : row) {
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
}