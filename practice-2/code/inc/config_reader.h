#ifndef __CONFIG_READER_H
#define __CONFIG_READER_H

#include <string>

struct Config {
    long int seed;
    int max_evaluaciones;
    int num_ejecuciones;
    double lambda;
    double ls_ratio;
    
    // Parameters for the custom market
    int use_custom_market;
    std::string custom_name;
    std::string custom_path;
    double custom_lo;
    double custom_hi;
};

class ConfigReader {
public:
    static Config read(const std::string& filename);
};

#endif