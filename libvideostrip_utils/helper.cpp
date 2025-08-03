/**
 * @file helper.cpp
 * @author Jose Cappelletto (cappelletto@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2020-07-03
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _PROJECT_HELPER_CPP_
#define _PROJECT_HELPER_CPP_

#include "headers.h"
#include "helper.h"
using namespace std;

/**
 * @brief Convert OpenCV data type into human readable format (e.g. CV32FC1)
 * 
 * @param type 
 * @return std::string 
 */
std::string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

/**
 * @brief Format integer argument into a fixed size string
 * 
 * @param i value to be formatted
 * @param length desired string lenght
 * @return std::string resulting zero-padded string
 */
std::string makeFixedLength(const int i, const int length)
{
    std::ostringstream ostr;
    if (i < 0)
        ostr << '-';

    ostr << std::setfill('0') << std::setw(length) << (i < 0 ? -i : i);
    return ostr.str();
}

/**
 * @brief Small formatted thread safe console logger. It provides a safe interface to generate formatted console output
 * There are error, debug, warning and info levels. Publisher tag is added for easier message tracing
 */
namespace logger{
  /**
   * @brief core function that formats the publisher field according the message type. Also, provides mutex protection for 
   * thread safety of the console output
   * 
   * @param type one of the supported message types:MSG_ERROR, MSG_DEBUG, MSG_WARNING, MSG_INFO 
   * @param publisher name of the message publisher
   * @param message string to be displayed
   * @return string returns a copy of the formatted string. It can be used for disk-based logging
   */
  string ConsoleOutput::publish(LogLevel type, std::string publisher, std::string message){
    std::ostringstream out;

    // thread_safe_log log = safe_cout();
    this->mtx.lock();
    switch(type){
      case logger::LogLevel::MSG_ERROR:
        out << red << "[error]" << light_cyan << " <" << publisher << "> " << reset << message << reset << endl;
        break;
      case logger::LogLevel::MSG_DEBUG:
        out << green << "[debug]" << light_cyan << " <" << publisher << "> " << reset << message << reset << endl;
        break;
      case logger::LogLevel::MSG_WARNING:
        out << yellow << "[warn] " << light_cyan << " <" << publisher << "> " << reset << message << reset << endl;
        break;
      case logger::LogLevel::MSG_INFO:
        out << reset << "[info] " << light_cyan << " <" << publisher << "> " << reset << message << reset << endl;
        break;
      default:
        out << reset << "[-]" << light_cyan << " <" << publisher << "> " << reset << message << reset << endl;
        break;
    }
    cout << out.str();
    this->mtx.unlock();
    return out.str();
  }

  /**
   * @brief Generates an error message
   * 
   * @param publisher nametag of who/where the message is being published 
   * @param message message string. It can contain extra decorators using supported escape sequences
   * @return string 
   */
  string ConsoleOutput::error(string publisher, string message){
    return ConsoleOutput::publish(LogLevel::MSG_ERROR, publisher, message);
  }

  /**
   * @brief Generates a debug message
   * 
   * @param publisher nametag of who/where the message is being published 
   * @param message message string. It can contain extra decorators using supported escape sequences
   * @return string 
   */
  string ConsoleOutput::debug(string publisher, string message){
    return ConsoleOutput::publish(LogLevel::MSG_DEBUG, publisher, message);
  }

  /**
   * @brief Generates a warning message
   * 
   * @param publisher nametag of who/where the message is being published 
   * @param message message string. It can contain extra decorators using supported escape sequences
   * @return string 
   */
  string ConsoleOutput::warn(string publisher, string message){
    return ConsoleOutput::publish(LogLevel::MSG_WARNING, publisher, message);
  }

  /**
   * @brief Generates an informative message
   * 
   * @param publisher nametag of who/where the message is being published 
   * @param message message string. It can contain extra decorators using supported escape sequences
   * @return string 
   */
  string ConsoleOutput::info(string publisher, string message){
    return ConsoleOutput::publish(LogLevel::MSG_INFO, publisher, message);
  }

  /**
   * @brief Overloaded implementation supporting ostringstream
   * 
   * @param publisher nametag of who/where the message is being published 
   * @param message message string. It can contain extra decorators using supported escape sequences
   * @return string 
   */
  string ConsoleOutput::error(string publisher, ostringstream &message){
    string r = ConsoleOutput::publish(LogLevel::MSG_ERROR, publisher, message.str());
    message.str("");
    return r;
  }

  /**
   * @brief Overloaded implementation supporting ostringstream
   * 
   * @param publisher nametag of who/where the message is being published 
   * @param message message string. It can contain extra decorators using supported escape sequences
   * @return string 
   */
  string ConsoleOutput::debug(string publisher, ostringstream &message){
    string r = ConsoleOutput::publish(LogLevel::MSG_DEBUG, publisher, message.str());
    message.str("");
    return r;
  }

  /**
   * @brief Overloaded implementation supporting ostringstream
   * 
   * @param publisher nametag of who/where the message is being published 
   * @param message message string. It can contain extra decorators using supported escape sequences
   * @return string 
   */
  string ConsoleOutput::warn(string publisher, ostringstream &message){
    string r =  ConsoleOutput::publish(LogLevel::MSG_WARNING, publisher, message.str());
    message.str("");
    return r;
  }

  /**
   * @brief Overloaded implementation supporting ostringstream
   * 
   * @param publisher nametag of who/where the message is being published 
   * @param message message string. It can contain extra decorators using supported escape sequences
   * @return string 
   */
  string ConsoleOutput::info(string publisher, ostringstream &message){
    string r =  ConsoleOutput::publish(LogLevel::MSG_INFO, publisher, message.str());
    message.str("");
    return r;
  }
}

#endif //_PROJECT_HELPER_CPP_