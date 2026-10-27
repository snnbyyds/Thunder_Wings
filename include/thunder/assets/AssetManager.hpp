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

#include "thunder/assets/AssetCatalog.hpp"
#include "thunder/game/Types.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <array>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <vector>

namespace thunder {

class AssetLoadError : public std::runtime_error {
public:
    explicit AssetLoadError(const std::string &message) : std::runtime_error(message) {}
};

/// Owns every SFML resource used by the game. The manager is an application
/// member; no resource is stored in a global.
class AssetManager {
public:
    AssetManager(std::filesystem::path assetsDirectory, bool enableAudio);

    AssetManager(const AssetManager &) = delete;
    AssetManager &operator=(const AssetManager &) = delete;

    [[nodiscard]] const sf::Texture &texture(TextureId id) const;
    [[nodiscard]] const sf::Font &font(FontId id) const;

    [[nodiscard]] bool audioEnabled() const {
        return audioEnabled_;
    }

    void playSound(SoundId sound);
    void playMusic();
    void pauseMusic();
    void resumeMusic();

private:
    static constexpr std::size_t kTextureCount = static_cast<std::size_t>(TextureId::Count);
    static constexpr std::size_t kFontCount = static_cast<std::size_t>(FontId::Count);
    static constexpr std::size_t kSoundCount = static_cast<std::size_t>(SoundId::Count);
    static constexpr std::size_t kMaxVoices = 16;

    std::filesystem::path assetsDirectory_;
    bool audioEnabled_{};
    std::array<sf::Texture, kTextureCount> textures_{};
    std::array<sf::Font, kFontCount> fonts_{};
    std::array<sf::SoundBuffer, kSoundCount> soundBuffers_{};
    std::vector<std::unique_ptr<sf::Sound>> voices_;
    sf::Music music_;
    bool musicLoaded_{};
};

} // namespace thunder
