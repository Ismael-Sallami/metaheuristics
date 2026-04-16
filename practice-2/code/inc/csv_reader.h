#ifndef __CSV_READER_H
#define __CSV_READER_H

#include <vector>
#include <string>

class CsvReader {
public:
    struct Data {
        std::vector<std::string> assetNames;
        std::vector<double> means;
        std::vector<std::vector<double>> covariance;
        int numAssets;
        int numDays;
        
        std::vector<double> logBenefits;
    };

    static Data read(const std::string& filename, bool is_2025);
};

#endif