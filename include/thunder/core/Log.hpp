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

#include <format>
#include <source_location>
#include <string>

namespace thunder {

enum class LogLevel { Info, Warning, Error };

void logMessage(LogLevel level, const std::source_location &location, std::string message);

} // namespace thunder

#define TW_LOG_INFO(...)                                                                           \
    ::thunder::logMessage(::thunder::LogLevel::Info, std::source_location::current(),              \
                          std::format(__VA_ARGS__))

#define TW_LOG_WARNING(...)                                                                        \
    ::thunder::logMessage(::thunder::LogLevel::Warning, std::source_location::current(),           \
                          std::format(__VA_ARGS__))

#define TW_LOG_ERROR(...)                                                                          \
    ::thunder::logMessage(::thunder::LogLevel::Error, std::source_location::current(),             \
                          std::format(__VA_ARGS__))
