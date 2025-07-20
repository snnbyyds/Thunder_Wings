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

#include "Core/Logging.hpp"

namespace logging {

static bool initialized = false;
static boost::shared_ptr<sinks::asynchronous_sink<sinks::text_ostream_backend>>
    asyncSink;

void init() {
    if (initialized)
        return;

    boost::log::add_common_attributes(); // TimeStamp, ProcessID, ThreadID,
                                         // LineID

    using async_console_sink =
        sinks::asynchronous_sink<sinks::text_ostream_backend>;

    auto backend = boost::make_shared<sinks::text_ostream_backend>();
    backend->add_stream(
        boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()));
    backend->auto_flush(true);

    asyncSink = boost::make_shared<async_console_sink>(backend);
    asyncSink->set_formatter(
        expr::stream << "["
                     << expr::format_date_time<boost::posix_time::ptime>(
                            "TimeStamp", "%Y-%m-%d %H:%M:%S")
                     << "] [" << trivial::severity << "] ["
                     << expr::attr<std::string>("File") << ":"
                     << expr::attr<int>("Line") << "] ["
                     << expr::attr<std::string>("Function") << "] "
                     << expr::smessage);

    boost::log::core::get()->set_filter(trivial::severity >= trivial::info);
    boost::log::core::get()->add_sink(asyncSink);

    initialized = true;
}

void setLogLevel(LogLevel level) {
    trivial::severity_level boostLevel =
        static_cast<trivial::severity_level>(level);
    boost::log::core::get()->set_filter(trivial::severity >= boostLevel);
}

void shutdown() {
    if (asyncSink) {
        asyncSink->stop();
        asyncSink->flush();
    }
    boost::log::core::get()->remove_all_sinks();
    initialized = false;
}

std::string extractFileName(const char *filePath) {
    std::string path(filePath);
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
        return path.substr(pos + 1);
    return path;
}

void logMessage(LogLevel level, const std::string &message, const char *file,
                int line, const char *function) {
    // Lazy initialization
    if (!initialized)
        init();

    std::string filename = extractFileName(file);
    src::severity_logger<trivial::severity_level> logger;

    BOOST_LOG_SCOPED_LOGGER_ATTR(logger, "File",
                                 attrs::constant<std::string>(filename));
    BOOST_LOG_SCOPED_LOGGER_ATTR(logger, "Line", attrs::constant<int>(line));
    BOOST_LOG_SCOPED_LOGGER_ATTR(logger, "Function",
                                 attrs::constant<std::string>(function));

    trivial::severity_level boostLevel =
        static_cast<trivial::severity_level>(level);
    BOOST_LOG_SEV(logger, boostLevel) << message;
}

} // namespace logging
