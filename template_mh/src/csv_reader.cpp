#include "csv_reader.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>


CsvReader::Data CsvReader::read(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    // Read header
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty CSV file");
    }

    std::stringstream ss(line);
    std::string cell;
    std::vector<std::string> assetNames;
    std::getline(ss, cell, ','); // Skip "Date"
    while (std::getline(ss, cell, ',')) {
        // Trim whitespace/quotes if necessary
        assetNames.push_back(cell);
    }

    int numAssets = assetNames.size();
    std::vector<std::vector<double>> returns; // returns[day][asset]

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream lineSS(line);
        std::getline(lineSS, cell, ','); // Skip "Date"
        std::vector<double> dayReturns;
        while (std::getline(lineSS, cell, ',')) {
            dayReturns.push_back(std::stod(cell));
        }
        if (dayReturns.size() == numAssets) {
            returns.push_back(dayReturns);
        }
    }

    int numDays = returns.size();
    Data data;
    data.assetNames = assetNames;
    data.numAssets = numAssets;
    data.numDays = numDays;
    data.means.resize(numAssets, 0.0);
    
    data.logBenefits.resize(numAssets, 0.0);

    // Calculate Means
    for (int j = 0; j < numAssets; ++j) {
        double sum = 0;
        double sum_log = 0; 
        for (int i = 0; i < numDays; ++i) {
            sum += returns[i][j];
            sum_log += std::log(1 + returns[i][j]); 
        }
        data.means[j] = sum / numDays;
        data.logBenefits[j] = sum_log / numDays; 
    }

    // Calculate Covariance Matrix
    data.covariance.resize(numAssets, std::vector<double>(numAssets, 0.0));
    for (int j = 0; j < numAssets; ++j) {
        for (int k = j; k < numAssets; ++k) {
            double cov = 0;
            for (int i = 0; i < numDays; ++i) {
                cov += (returns[i][j] - data.means[j]) * (returns[i][k] - data.means[k]);
            }
            cov /= (numDays);
            data.covariance[j][k] = cov;
            data.covariance[k][j] = cov;
        }
    }

    return data;
}

