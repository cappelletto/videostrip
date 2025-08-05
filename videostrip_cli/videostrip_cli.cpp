/**
 * @file main.cpp
 * @author Jose Cappelletto (cappelletto@gmail.com)
 * @brief [videostrip] as stand-alone module for video processing. Rework from scracth, based on the original uwimgproc/videostrip.cpp
 * @version 3.6-DualEnv [local + Iridis5]
 * @date 2021-11-18
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */
#include <iostream>
#include <args.hxx>                       // from external/

#include <headers.hpp>     // was headers.h
#include <options.hpp>     // was options.h
#include <helper.hpp>      // helper moved into core

using namespace std;
using namespace cv;

/*!
    @fn     int main(int argc, char* argv[])
    @brief  Main function
*/

logger::ConsoleOutput logc; // as a global variable, we are Ok with this

int main(int argc, char *argv[])
{

    logc.warn("main", "Alpha version. Sandbox for module building and testing");

    int retval = initParser(argc, argv);    // initial argument validation, populates arg parsing structure args
    if (retval != 0)                        // some error ocurred, we have been signaled to stop
        return retval;
    std::ostringstream s;

    // Input file priority: must be defined either by the config.yaml or --input argument
    string inputFileName    = ""; // command arg or config defined
    string outputFileName   = ""; // if empty, output filenames will be the same as the standard. If non-null, will be used as prefix
    string outputFilePath   = ""; // absolut/relative folder path were output will be stored
    int verbosityLevel      = 0;  // verbosity level, 0 - 3

    if (argInput)   inputFileName    = args::get(argInput);   //input file is mandatory positional argument. Overrides any definition in configuration.yaml
    if (argOutput)  outputFileName   = args::get(argOutput);  //input file is mandatory positional argument. Overrides any definition in configuration.yaml
    if (argVerbose) verbosityLevel   = args::get(argVerbose); // retrieve user defined verbosity level

    if (argDumpInfo)
    {
        std::cout << "\tOpenCV version:\t" << yellow << CV_VERSION << reset << endl;
        // cout << "\tGit commit:\t" << yellow << GIT_COMMIT << reset << endl;
        // cout << "\tMode:\t\t" << yellow << CMAKE_BUILD_TYPE << reset << endl;
        std::cout << cv::getBuildInformation() << std::endl;
        s << "Input file: " << inputFileName << endl;
        s << "Output file: " << outputFileName << endl;
        s << "Verbosity level: " << verbosityLevel << endl;
        logc.info("main", s.str());
        return 0;
    }

    if (inputFileName.empty()){ //not defined as command line argument? let's use config.yaml definition
        logc.error ("main", "Input file missing. Please define it using --input=<filename>");
        return -1;
    }

    return 0;
}
