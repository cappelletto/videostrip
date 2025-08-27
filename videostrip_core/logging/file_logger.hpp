// videostrip_core/logging/file_logger.hpp
#pragma once
#include <videostrip_core/logging/logger.hpp>
#include <fstream>
#include <mutex>


// TODO: implement log rotation, max file size (optional)
// TODO: Add timestamp to each log entry
// TODO: Promote filename as static member or constructor argument so we do not need to open/close file each time (time consuming)
// This would required some file opening/closing methods and validation
namespace videostrip::logger {

class FileLogger : public ::videostrip::Logger {
public:
    explicit FileLogger(const std::string& path)
    : ofs_(path, std::ios::app) {}

    void info (const std::string& msg) override { write("INFO",  msg); }
    void warn (const std::string& msg) override { write("WARN",  msg); }
    void error(const std::string& msg) override { write("ERROR", msg); }
    void debug(const std::string& msg) override { write("DEBUG", msg); }

private:
    std::ofstream ofs_;
    std::mutex mtx_;

    void write(const char* level, const std::string& msg) {
        if (!ofs_) return;
        std::lock_guard<std::mutex> lock(mtx_);
        ofs_ << "[" << level << "] " << msg << '\n';
        ofs_.flush();
    }
};

} // namespace videostrip::logger
