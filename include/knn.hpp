#ifndef KNN_HPP
#define KNN_HPP

#include "parser.hpp"
#include <vector>
#include <string>
#include <string_view>

class Knn {

public:
    Knn();
    ~Knn();

private:
    CsvParser _parser;
    const std::vector<std::vector<std::string_view>>* _data;
};

#endif // KNN_HPP
