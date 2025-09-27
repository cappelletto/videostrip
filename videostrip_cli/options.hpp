/**
 * @file options.hpp
 * @brief Argument parser options based on args.hxx. Extended to accomodate multiple modules using
 * similar parsers
 * @version 1.1
 * @date 18/06/2020
 * @author Jose Cappelletto
 */

#ifndef _PROJECT_OPTIONS_H_
#define _PROJECT_OPTIONS_H_

#include <headers.hpp>
#include <iostream>

args::ArgumentParser argParser("", "");
args::HelpFlag argHelp(argParser, "help", "Display this help menu", {'h', "help"});

args::Flag argDumpInfo(argParser, "dump", "Show compilation time related information",
                       {'d', "dump"});
args::ValueFlag<std::string> argInput(argParser, "input", "Path to video input file", {"input"});
args::ValueFlag<std::string> argOutput(argParser, "output", "Output filename prefix",
                                       {'o', "output"});
args::ValueFlag<int> argVerbose(argParser, "verbose", "Define verbosity level, 0 - 3", {"verbose"});

// Free parameters for debugging
args::ValueFlag<int> argIntParam(argParser, "param",
                                 "User defined parameter INTEGER for testing purposes", {"int"});
args::ValueFlag<float> argFloatParam(argParser, "param",
                                     "User defined parameter FLOAT for testing purposes",
                                     {"float"});

/**
 * @brief Default initializer for argument parsing object
 *
 * @param argc cli argc (count)
 * @param argv cli argv (value)
 * @return int error code if any
 */
int initParser(int argc, char* argv[])
{
    /* PARSER section */
    std::string descriptionString = "videostrip - Complete description \
    OpenCV GPU C++17 multithreaded video processing";

    argParser.Description(descriptionString);
    argParser.Epilog("Author: J. Cappelletto (GitHub: @cappelletto)\n");
    argParser.Prog(argv[0]);
    argParser.helpParams.width = 120;

    try
    {
        argParser.ParseCLI(argc, argv);
    }
    catch (const args::Completion& e)
    {
        std::cout << e.what();
        return 0;
    }

    catch (args::Help)
    { // if argument asking for help, show this message
        std::cout << argParser;
        return -1;
    }
    catch (args::ParseError e)
    { // if some error ocurr while parsing, show summary
        std::cerr << e.what() << std::endl;
        std::cerr << "Use -h, --help command to see usage" << std::endl;
        return -1;
    }
    catch (args::ValidationError e)
    { // if some error at argument validation, show
        std::cerr << "Bad input commands" << std::endl;
        std::cerr << "Use -h, --help command to see usage" << std::endl;
        return -1;
    }
    std::cout << "\tBuilt:\t" << __DATE__ << " - " << __TIME__ << endl;
    return 0;
}

#endif //_PROJECT_OPTIONS_H_