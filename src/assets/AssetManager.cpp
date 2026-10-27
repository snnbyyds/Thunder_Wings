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

#include "thunder/assets/AssetManager.hpp"

#include "thunder/core/Log.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

namespace thunder {

AssetManager::AssetManager(std::filesystem::path assetsDirectory, bool enableAudio)
    : assetsDirectory_(std::move(assetsDirectory)), audioEnabled_(enableAudio) {
    std::string errors;

    for (std::size_t i = 0; i < kTextureCount; ++i) {
        const auto id = static_cast<TextureId>(i);
        const auto file = assetsDirectory_ / texturePath(id);
        if (!textures_[i].loadFromFile(file)) {
            errors += "texture '" + file.string() + "'\n";
            continue;
        }
        textures_[i].setSmooth(textureSmooth(id));
    }

    for (std::size_t i = 0; i < kFontCount; ++i) {
        const auto id = static_cast<FontId>(i);
        const auto file = assetsDirectory_ / fontPath(id);
        if (!fonts_[i].openFromFile(file))
            errors += "font '" + file.string() + "'\n";
    }

    if (audioEnabled_) {
        for (std::size_t i = 0; i < kSoundCount; ++i) {
            const auto id = static_cast<SoundId>(i);
            const auto file = assetsDirectory_ / soundPath(id);
            if (!soundBuffers_[i].loadFromFile(file))
                errors += "sound '" + file.string() + "'\n";
        }

        const auto musicFile = assetsDirectory_ / musicPath();
        musicLoaded_ = music_.openFromFile(musicFile);
        if (!musicLoaded_) {
            errors += "music '" + musicFile.string() + "'\n";
        } else {
            music_.setLooping(true);
            music_.setVolume(8.0f);
        }
    }

    if (!errors.empty())
        throw AssetLoadError("Failed to load assets:\n" + errors);

    voices_.reserve(kMaxVoices);
}

const sf::Texture &AssetManager::texture(TextureId id) const {
    return textures_.at(static_cast<std::size_t>(id));
}

const sf::Font &AssetManager::font(FontId id) const {
    return fonts_.at(static_cast<std::size_t>(id));
}

void AssetManager::playSound(SoundId sound) {
    if (!audioEnabled_)
        return;

    const std::size_t index = static_cast<std::size_t>(sound);
    if (index >= soundBuffers_.size())
        return;

    auto available =
        std::find_if(voices_.begin(), voices_.end(), [](const std::unique_ptr<sf::Sound> &voice) {
            return voice->getStatus() == sf::SoundSource::Status::Stopped;
        });

    try {
        if (available == voices_.end()) {
            if (voices_.size() >= kMaxVoices)
                return;
            voices_.push_back(std::make_unique<sf::Sound>(soundBuffers_[index]));
            available = std::prev(voices_.end());
        }
        (*available)->setBuffer(soundBuffers_[index]);
        (*available)->play();
    } catch (const std::exception &error) {
        TW_LOG_WARNING("Audio playback failed: {}", error.what());
        audioEnabled_ = false;
    }
}

void AssetManager::playMusic() {
    if (!audioEnabled_ || !musicLoaded_)
        return;
    if (music_.getStatus() != sf::SoundSource::Status::Playing)
        music_.play();
}

void AssetManager::pauseMusic() {
    if (audioEnabled_ && musicLoaded_)
        music_.pause();
}

void AssetManager::resumeMusic() {
    playMusic();
}

} // namespace thunder
