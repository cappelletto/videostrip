/**
 * @file logger.hpp
 * @brief Thread-safe console logger for videostrip_core
 * @version 1.0
 * @date 2025-08-14
 */

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <mutex>

namespace videostrip
{
    // Interface lives in the videostrip namespace (as per videostrip_core.hpp)
    class Logger
    {
    public:
        virtual ~Logger() = default;
        virtual void info(const std::string &msg) = 0;
        virtual void warn(const std::string &msg) = 0;
        virtual void error(const std::string &msg) = 0;
        virtual void debug(const std::string &msg) = 0;
    };

    enum class LogLevel : unsigned int
    {
        MSG_INFO    = 1,    ///< Standard informative message
        MSG_WARNING = 2,    ///< Non-critical warning message
        MSG_DEBUG   = 3,    ///< Debug (verbose) message
        MSG_ERROR   = 4     ///< Critical error message
    };

    // Implementation namespace for concrete loggers
    namespace logger
    {
        /**
         * @class ConsoleLogger
         * @brief Simple console logger implementation of Logger class
         *
         * Supports optional publisher tags and thread-safe output to std::cout.
         */
        class ConsoleLogger : public ::videostrip::Logger
        {
        public:
            explicit ConsoleLogger(const std::string &publisher = "core")
                : m_publisher(publisher) {}

            // default methods from Logger interface, uses default tag
            void info(const std::string &msg) override  { publish("INFO",  msg); }
            void warn(const std::string &msg) override  { publish("WARN",  msg); }
            void error(const std::string &msg) override { publish("ERROR", msg); }
            void debug(const std::string &msg) override { publish("DEBUG", msg); }

            // Optional overloads with explicit publisher tag
            void info (const std::string &publisher, const std::string &msg) { publish("INFO",  msg, publisher); }
            void warn (const std::string &publisher, const std::string &msg) { publish("WARN",  msg, publisher); }
            void error(const std::string &publisher, const std::string &msg) { publish("ERROR", msg, publisher); }
            void debug(const std::string &publisher, const std::string &msg) { publish("DEBUG", msg, publisher); }

        private:
            std::string m_publisher;    // publisher tag for log messages
            std::mutex mtx;             // mutex for thread-safe output   

            void publish(const std::string &level, const std::string &msg, const std::string &publisher = "")
            {
                std::lock_guard<std::mutex> lock(mtx);
                const std::string &tag = publisher.empty() ? m_publisher : publisher;
                std::cout << "[" << level << "] <" << tag << "> " << msg << std::endl;
            }
        };
    } // namespace logger
} // namespace videostrip
