#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/json/parser.hpp"
#include <fstream>
#include <string>
#include <vector>

static std::vector<std::string> data_files {
      "data/canada.json"
    , "data/citm_catalog.json"
    , "data/twitter.json"
};

TEST_CASE("Parse files") {
    
    bool result = std::all_of(data_files.cbegin()
            , data_files.cend()
            , [] (std::string const & filename) {
        std::cout << "Parsing file: " << filename << "\n";
        
        std::ifstream ifs(filename, std::ios::binary);
        
        if (!ifs.is_open()) {
            std::cerr << "Open file " << filename << " failure\n";
            return false;
        }
        
        return true;
    });
    
    CHECK_MESSAGE(result, "Parse files tested successfuly");
}

