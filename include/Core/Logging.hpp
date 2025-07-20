/*
 * Copyright 2025 Nuo Shen, Nanjing University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Macros.h"
#include <cstdint>
// NOLINTBEGIN(llvmlibc-restrict-system-libc-headers)
#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
// NOLINTEND(llvmlibc-restrict-system-libc-headers)
#include <ostream> // IWYU pragma: keep
#include <string>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // deprecated functions
#pragma warning(disable : 4251) // dll interface
#endif

namespace logging {
namespace sinks = boost::log::sinks;       // NOLINT(misc-unused-alias-decls)
namespace expr = boost::log::expressions;  // NOLINT(misc-unused-alias-decls)
namespace attrs = boost::log::attributes;  // NOLINT(misc-unused-alias-decls)
namespace keywords = boost::log::keywords; // NOLINT(misc-unused-alias-decls)
namespace trivial = boost::log::trivial;
namespace src = boost::log::sources; // NOLINT(misc-unused-alias-decls)

enum class LogLevel : uint8_t {
    Info = trivial::info,
    Warning = trivial::warning,
    Error = trivial::error
};

void init();

// Set log level
void setLogLevel(LogLevel level);

// Shutdown logging system
void shutdown();

// Get filename from filepath
std::string extractFileName(const char *filePath);

void logMessage(LogLevel level, const std::string &message, const char *file,
                int line, const char *function);

} // namespace logging

// Macros
#define LOG_INFO(msg)                                                          \
    do {                                                                       \
        std::ostringstream _log_oss;                                           \
        _log_oss << msg;                                                       \
        logging::logMessage(logging::LogLevel::Info, _log_oss.str(),           \
                            GAME_LOG_FILE, GAME_LOG_LINE, GAME_LOG_FUNCTION);  \
    } while (0)

#define LOG_WARN(msg)                                                          \
    do {                                                                       \
        std::ostringstream _log_oss;                                           \
        _log_oss << msg;                                                       \
        logging::logMessage(logging::LogLevel::Warning, _log_oss.str(),        \
                            GAME_LOG_FILE, GAME_LOG_LINE, GAME_LOG_FUNCTION);  \
    } while (0)

#define LOG_ERROR(msg)                                                         \
    do {                                                                       \
        std::ostringstream _log_oss;                                           \
        _log_oss << msg;                                                       \
        logging::logMessage(logging::LogLevel::Error, _log_oss.str(),          \
                            GAME_LOG_FILE, GAME_LOG_LINE, GAME_LOG_FUNCTION);  \
    } while (0)

#ifdef _MSVC_VER
#pragma warning(pop)
#endif
