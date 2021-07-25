#include<iostream>
#include"boardconfig.hpp"
#include<boost/program_options.hpp>

namespace po = boost::program_options;

class Config
{
public:
    static Config fromCommandLine(int argc, char *argv[]);
    void setSize(const std::string &size);
    bool fullscreen;
    unsigned int width;
    unsigned int height; 
    BoardConfig board;
};

std::ostream& operator<<(std::ostream& os, const Config& config);

po::options_description createOptionsDescription(BoardConfig &config,
    std::string &rulesString, std::string &boardSize, std::string &windowSize,
    std::string &colours);
void printHelpAndExit(const po::options_description &desc);