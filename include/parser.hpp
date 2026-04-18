#include <fstream>

class CsvParser {
public:
    CsvParser(const std::string& filename);
    ~CsvParser();
    bool parse();
    void print();

private:
    std::string _filename;
    std::vector<std::vector<std::string>> _data;
};