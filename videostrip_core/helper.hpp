/**
 * @file    helper.hpp
 * @author  Jose Cappelletto (cappelletto@gmail.com)
 * @brief   Collection of general helper functions
 * @version 0.2.1
 * @date    2020-07-03
 * 
 * @copyright Copyright (c) 2020-2025
 * 
 */

#ifndef _PROJECT_HELPER_HPP_
#define _PROJECT_HELPER_HPP_

#pragma once

#include <mutex>
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;

// escape based colour codes for console output
const std::string red("\033[1;31m");
const std::string green("\033[1;32m");
const std::string yellow("\033[1;33m");
const std::string blue("\033[1;34m");
const std::string purple("\033[1;35m");
const std::string cyan("\033[1;36m");

const std::string light_red("\033[0;31m");
const std::string light_green("\033[0;32m");
const std::string light_yellow("\033[0;33m");
const std::string light_blue("\033[0;34m");
const std::string light_purple("\033[0;35m");
const std::string light_cyan("\033[0;36m");

const std::string reset("\033[0m");
const std::string highlight("\033[30;43m");

std::string type2str(int type);
std::string makeFixedLength(const int i, const int length);

/**
 * @brief logger class that provides thread safe cout output to the console, with additional colour-coded formatting
 * 
 */

namespace logger{
    enum class LogLevel : unsigned int{
        MSG_INFO    = 1,    // Standard informative message
        MSG_WARNING = 2,    // non-critical warning message
        MSG_DEBUG   = 3,    // debug (verbose) message
        MSG_ERROR   = 4     // critical error message
    };

    class ConsoleOutput{
        private:
            // std::vector<std::string> logHistory; // history of all received messages
            // int counter; //number of calls to publish. It should match logHistory size
            std::mutex mtx;
        protected:
        public:
            ConsoleOutput(){
                // counter = 0; //
            };
            ~ConsoleOutput(){
                // this->logHistory.clear();
            };

            string publish(logger::LogLevel type, std::string owner, std::string message);
            string publisher(string name);

            string error(string owner, string message);
            string warn (string owner, string message);
            string debug(string owner, string message);
            string info (string owner, string message);

            string error(string owner, ostringstream &message);
            string warn (string owner, ostringstream &message);
            string debug(string owner, ostringstream &message);
            string info (string owner, ostringstream &message);

            void clear(); // clear the history log
            int  size();  // return the number of log entries
            void dump();  // dump (on screen or file) the log
    };

};
#endif // _PROJECT_HELPER_HPP_