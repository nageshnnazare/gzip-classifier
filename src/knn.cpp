#include "knn.hpp"

Knn::Knn() : _parser("datasets/ag_news/test.csv") {
    _parser.parseData();
    _data = &_parser.getData();
    _parser.printData(10);
}

Knn::~Knn() {}
