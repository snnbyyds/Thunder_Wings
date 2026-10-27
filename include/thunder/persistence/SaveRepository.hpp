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

#include "thunder/persistence/SaveData.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace thunder {

class SaveRepository {
public:
    explicit SaveRepository(std::filesystem::path path);

    [[nodiscard]] static std::optional<std::filesystem::path> defaultPath();

    [[nodiscard]] const std::filesystem::path &path() const {
        return path_;
    }

    [[nodiscard]] bool exists() const;

    [[nodiscard]] std::optional<SaveGame> load(std::string &error) const;
    bool save(const SaveGame &game, std::string &error) const;

private:
    [[nodiscard]] std::filesystem::path backupPath() const;
    [[nodiscard]] std::filesystem::path temporaryPath() const;
    [[nodiscard]] std::optional<SaveGame> loadFile(const std::filesystem::path &path,
                                                   std::string &error) const;

    std::filesystem::path path_;
};

} // namespace thunder
