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

#include "thunder/persistence/SaveRepository.hpp"

#include "thunder/core/Log.hpp"
#include "thunder/persistence/SaveCodec.hpp"

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <system_error>
#include <utility>

namespace thunder {
namespace {

constexpr std::uintmax_t kMaximumSaveSize = 16ULL * 1024ULL * 1024ULL;

std::optional<std::filesystem::path> environmentPath(const char *name) {
    const char *value = std::getenv(name);
    if (value == nullptr || *value == '\0')
        return std::nullopt;
    return std::filesystem::path(value);
}

} // namespace

SaveRepository::SaveRepository(std::filesystem::path path) : path_(std::move(path)) {}

std::optional<std::filesystem::path> SaveRepository::defaultPath() {
#ifdef _WIN32
    const auto base = environmentPath("APPDATA");
    const std::filesystem::path appDirectory = "ThunderWings";
#else
    auto base = environmentPath("XDG_CONFIG_HOME");
    if (!base) {
        const auto home = environmentPath("HOME");
        if (!home)
            return std::nullopt;
        base = *home / ".config";
    }
    const std::filesystem::path appDirectory = "thunder-wings";
#endif
    if (!base)
        return std::nullopt;
    return *base / appDirectory / "game_data.json";
}

std::filesystem::path SaveRepository::backupPath() const {
    return std::filesystem::path(path_.string() + ".bak");
}

std::filesystem::path SaveRepository::temporaryPath() const {
    return std::filesystem::path(path_.string() + ".tmp");
}

bool SaveRepository::exists() const {
    std::error_code error;
    return std::filesystem::exists(path_, error) || std::filesystem::exists(backupPath(), error);
}

std::optional<SaveGame> SaveRepository::loadFile(const std::filesystem::path &path,
                                                 std::string &error) const {
    std::error_code filesystemError;
    const std::uintmax_t size = std::filesystem::file_size(path, filesystemError);
    if (filesystemError) {
        error = "Cannot stat save file '" + path.string() + "': " + filesystemError.message();
        return std::nullopt;
    }
    if (size > kMaximumSaveSize) {
        error = "Save file is too large: " + path.string();
        return std::nullopt;
    }

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        error = "Cannot open save file: " + path.string();
        return std::nullopt;
    }
    std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (!input.good() && !input.eof()) {
        error = "Cannot read save file: " + path.string();
        return std::nullopt;
    }

    try {
        return decodeSave(contents);
    } catch (const SaveError &saveError) {
        // The UI deliberately sees the same message for old, malformed and
        // corrupted files; the detailed reason remains in the log.
        TW_LOG_WARNING("Cannot load {}: {}", path.string(), saveError.what());
        error = "Invalid or corrupted save file";
        return std::nullopt;
    }
}

std::optional<SaveGame> SaveRepository::load(std::string &error) const {
    error.clear();
    std::string primaryError;
    if (auto save = loadFile(path_, primaryError))
        return save;

    std::string backupError;
    if (auto save = loadFile(backupPath(), backupError)) {
        TW_LOG_WARNING("Primary save failed ({}); loaded backup {}", primaryError,
                       backupPath().string());
        return save;
    }

    std::error_code existenceError;
    const bool primaryExists = std::filesystem::exists(path_, existenceError);
    if (!primaryExists && !std::filesystem::exists(backupPath(), existenceError))
        error = "No saved game found";
    else if (primaryError.empty())
        error = "No saved game found";
    else
        error = primaryError;
    return std::nullopt;
}

bool SaveRepository::save(const SaveGame &game, std::string &error) const {
    error.clear();

    std::error_code filesystemError;
    const auto parent = path_.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent, filesystemError);
        if (filesystemError) {
            error = "Cannot create save directory '" + parent.string() +
                    "': " + filesystemError.message();
            return false;
        }
    }

    std::string encoded;
    try {
        encoded = encodeSave(game);
    } catch (const std::exception &exception) {
        error = std::string("Cannot encode save: ") + exception.what();
        return false;
    }

    const auto temporary = temporaryPath();
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output.is_open()) {
            error = "Cannot open temporary save file: " + temporary.string();
            return false;
        }
        output.write(encoded.data(), static_cast<std::streamsize>(encoded.size()));
        output.flush();
        if (!output.good()) {
            error = "Cannot write temporary save file: " + temporary.string();
            output.close();
            std::filesystem::remove(temporary, filesystemError);
            return false;
        }
    }

    // Keep the previous known-good file as a recovery point. A backup failure
    // must not prevent writing the new save.
    std::filesystem::copy_file(path_, backupPath(),
                               std::filesystem::copy_options::overwrite_existing, filesystemError);

    std::filesystem::rename(temporary, path_, filesystemError);
    if (filesystemError) {
        // Windows does not replace an existing destination with rename().
        std::filesystem::remove(path_, filesystemError);
        filesystemError.clear();
        std::filesystem::rename(temporary, path_, filesystemError);
    }
    if (filesystemError) {
        error = "Cannot replace save file '" + path_.string() + "': " + filesystemError.message();
        std::filesystem::remove(temporary, filesystemError);
        return false;
    }
    return true;
}

} // namespace thunder
