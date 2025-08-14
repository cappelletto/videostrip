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
    enum class LogLevel : unsigned int
    {
        MSG_INFO    = 1,    ///< Standard informative message
        MSG_WARNING = 2,    ///< Non-critical warning message
        MSG_DEBUG   = 3,    ///< Debug (verbose) message
        MSG_ERROR   = 4     ///< Critical error message
    };

    /// Minimal logger interface for file logging and in-memory debugging
    /**
     * @class Logger
     * @brief Minimal logger interface for file logging and in-memory debugging.
     *
     * Provides methods for info, warning, error, and debug messages.
     */
    class Logger
    {
    public:
        virtual ~Logger() = default;
        virtual void info(const std::string &msg) = 0;
        virtual void warn(const std::string &msg) = 0;
        virtual void error(const std::string &msg) = 0;
        virtual void debug(const std::string &msg) = 0;
    };

    /**
     * @class ConsoleLogger
     * @brief Simple console logger implementation of Logger.
     *
     * Supports optional publisher tags and thread-safe output to std::cout.
     */
    class ConsoleLogger : public Logger
    {
    public:
        ConsoleLogger(const std::string &publisher = "core")
            : m_publisher(publisher) {}

        void info(const std::string &msg) override
        {
            publish("INFO", msg);
        }
        void warn(const std::string &msg) override
        {
            publish("WARN", msg);
        }
        void error(const std::string &msg) override
        {
            publish("ERROR", msg);
        }
        void debug(const std::string &msg) override
        {
            publish("DEBUG", msg);
        }

        // Optionally, overloads with publisher tag
        void info(const std::string &publisher, const std::string &msg)
        {
            publish("INFO", msg, publisher);
        }
        void warn(const std::string &publisher, const std::string &msg)
        {
            publish("WARN", msg, publisher);
        }
        void error(const std::string &publisher, const std::string &msg)
        {
            publish("ERROR", msg, publisher);
        }
        void debug(const std::string &publisher, const std::string &msg)
        {
            publish("DEBUG", msg, publisher);
        }

    private:
        std::string m_publisher;
        std::mutex mtx;

        void publish(const std::string &level, const std::string &msg, const std::string &publisher = "")
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::string tag = publisher.empty() ? m_publisher : publisher;
            std::cout << "[" << level << "] <" << tag << "> " << msg << std::endl;
        }
    };
} // namespace videostrip_core::logger
