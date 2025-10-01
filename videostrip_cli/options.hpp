/**
 * @file options.hpp
 * @brief Argument parser options based on args.hxx. Extended to accommodate multiple modules using
 *        similar parsers. Adds --version / --print-schema handling.
 * @version 1.2
 * @date 2025-09-29
 * @author Jose Cappelletto
 */

#ifndef _PROJECT_OPTIONS_H_
#define _PROJECT_OPTIONS_H_

#include <headers.hpp>
#include <iostream>

// ---------------------------
// Helper structures
// ---------------------------
struct VersionOptions
{
    bool enabled = false;
    std::string format = "text"; // "text" | "json"
};

struct PrintSchemaOptions
{
    bool enabled = false;
    std::string which = "all";   // "summary" | "frames" | "all"
    std::string format = "yaml"; // "yaml" | "json"
    bool brief = false;
};

// Expose parsed options to callers (read-only after initParser)
inline VersionOptions gVersionOpts{};
inline PrintSchemaOptions gSchemaOpts{};

// ---------------------------
// Global parser and common flags
// ---------------------------
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

// ---------------------------
// Version / Schema reporting flags (Issue #65)
// ---------------------------
args::Flag argVersion(argParser, "version", "Print version and exit", {'V', "version"});
args::ValueFlag<std::string> argVersionFormat(argParser, "fmt", "Version format: text|json",
                                              {"version-format"});

args::Flag argPrintSchema(argParser, "print-schema", "Print output schema and exit",
                          {"print-schema"});
args::ValueFlag<std::string> argSchemaWhich(argParser, "which", "Which schema: summary|frames|all",
                                            {"schema-which"});
args::ValueFlag<std::string> argSchemaFormat(argParser, "format", "Schema format: yaml|json",
                                             {"schema-format"});
args::Flag argSchemaBrief(argParser, "brief", "Brief schema info", {"brief"});

/**
 * @brief Default initializer for argument parsing object
 *
 * @param argc cli argc (count)
 * @param argv cli argv (value)
 * @return int error code if any (0 ok, <0 handled help/parse cases)
 */
inline int initParser(int argc, char* argv[])
{
    /* PARSER section */
    std::string descriptionString =
        "videostrip - Video frame extraction tool based on feature detection and matching."
        "Extracts frames from a video file based on visual content, avoiding duplicates and "
        "near-duplicates."
        "Uses OpenCV for video processing and feature detection."
        "Licensed under the Apache License, Version 2.0 (the \"License\").";

    argParser.Description(descriptionString);
    argParser.Epilog("Author: J. Cappelletto (GitHub: @cappelletto)
");
    argParser.Prog(argv[0]);
    argParser.helpParams.width = 120;

    try {
        argParser.ParseCLI(argc, argv);
    } catch (const args::Completion& e) {
        std::cout << e.what();
        return 0;
    } catch (args::Help) {
        std::cout << argParser;
        return -1;
    } catch (args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "Use -h, --help command to see usage" << std::endl;
        return -1;
    } catch (args::ValidationError&) {
        std::cerr << "Bad input commands" << std::endl;
        std::cerr << "Use -h, --help command to see usage" << std::endl;
        return -1;
    }

    // Populate global reporting options
    gVersionOpts.enabled = argVersion;
    if (argVersionFormat) gVersionOpts.format = args::get(argVersionFormat);

    gSchemaOpts.enabled = argPrintSchema;
    if (argSchemaWhich)  gSchemaOpts.which  = args::get(argSchemaWhich);
    if (argSchemaFormat) gSchemaOpts.format = args::get(argSchemaFormat);
    gSchemaOpts.brief = argSchemaBrief;

    std::cout << "	Built:	" << __DATE__ << " - " << __TIME__ << std::endl;
    return 0;
}

#endif //_PROJECT_OPTIONS_H_
