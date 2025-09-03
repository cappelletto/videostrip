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

#include <iostream>
#include <fstream>
#include <mutex>
#include <sstream>

using std::ostringstream;

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

// TODO: promote as members of utility functions class
std::string type2str(int type);
std::string makeFixedLength(const int i, const int length);

/**
 * @brief logger class that provides thread safe cout output to the console, with additional colour-coded formatting
 *
 */

#endif // _PROJECT_HELPER_HPP_