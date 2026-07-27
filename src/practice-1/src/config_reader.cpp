#include "config_reader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Config ConfigReader::read(const std::string& filename) {
    // Set default values for the configuration
    Config config = {42, 10000, 50, 500.0, 0.4, 0, "Default", "", 0.0, 1.0};

    // Try to open the configuration file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << " [WARNING] File not found " << filename << ". Using defaults.\n";
        return config;
    }

    // Read the file line by line
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') continue; 
        
        // Parse the key-value pair separated by '='
        std::stringstream ss(line);
        std::string key, value;
        
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            // Map each key to its corresponding config value
            if (key == "SEED") config.seed = std::stol(value);
            else if (key == "MAX_EVALUACIONES") config.max_evaluaciones = std::stoi(value);
            else if (key == "NUM_EJECUCIONES") config.num_ejecuciones = std::stoi(value);
            else if (key == "LAMBDA") config.lambda = std::stod(value);
            else if (key == "LS_RATIO") config.ls_ratio = std::stod(value);
            // Parse custom market parameters
            else if (key == "USE_CUSTOM_MARKET") config.use_custom_market = std::stoi(value);
            else if (key == "CUSTOM_NAME") config.custom_name = value;
            else if (key == "CUSTOM_PATH") config.custom_path = value;
            else if (key == "CUSTOM_LO") config.custom_lo = std::stod(value);
            else if (key == "CUSTOM_HI") config.custom_hi = std::stod(value);
        }
    }
    return config;
}