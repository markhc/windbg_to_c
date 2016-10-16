#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include "windbg_structure.hpp"

int main(int argc, char** argv)
{
    if(argc < 2) {
        std::cout << "Usage: windbg_to_c.exe input.txt" << std::endl;
        return EXIT_FAILURE;
    }
    std::ifstream input(argv[1]);
    std::ofstream output("result.txt");
    
    std::stringstream inputstream;
    inputstream << input.rdbuf();
    
    windbg_structure s(inputstream.str());
    
    output << s.as_string(0) << std::endl;
    
    return EXIT_SUCCESS;
}