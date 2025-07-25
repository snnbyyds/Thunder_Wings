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

#include "Core/ResourceManager.hpp"
#include "Core/Logging.hpp"
#include <filesystem>

sf::Font ResourceManager::gameFont;
sf::Font ResourceManager::pageFont;
sf::Music ResourceManager::gameBackgroundMusic;

std::unordered_map<std::string, sf::Texture> ResourceManager::textures;

std::unordered_map<std::string, sf::SoundBuffer> ResourceManager::soundBuffers;
std::vector<std::unique_ptr<sf::Sound>> ResourceManager::activeSounds;

sf::Texture &ResourceManager::getTexture(const std::string &texturePath) {
    auto it = textures.find(texturePath);
    if (it != textures.end())
        return it->second;

    auto &texture = textures[texturePath];
    if (!texture.loadFromFile(texturePath)) {
        textures.erase(texturePath);
        throw TextureLoadException("Failed to load texture: " + texturePath);
    } else {
        LOG_INFO("Loaded texture: " + texturePath);
    }
    return texture;
}

bool ResourceManager::getTextureifExists(const std::string &texturePath) {
    auto it = textures.find(texturePath);
    if (it != textures.end())
        return true;

    if (!std::filesystem::exists(texturePath))
        return false;

    auto &texture = textures[texturePath];
    bool res = texture.loadFromFile(texturePath);
    if (res) {
        LOG_INFO("Loaded texture: " + texturePath);
        return true;
    }
    textures.erase(texturePath);
    return false;
}

void ResourceManager::loadGameFont(const std::string &fontPath) {
    if (!gameFont.loadFromFile(fontPath))
        throw FontLoadException("Failed to load font: " + fontPath);
    else
        LOG_INFO("Loaded font: " + fontPath + " as gameFont");
}

void ResourceManager::loadPageFont(const std::string &fontPath) {
    if (!pageFont.loadFromFile(fontPath))
        throw FontLoadException("Failed to load font: " + fontPath);
    else
        LOG_INFO("Loaded font: " + fontPath + " as pageFont");
}

void ResourceManager::loadBackgroundMusic(const std::string &filePath) {
    if (!gameBackgroundMusic.openFromFile(filePath))
        throw AudioLoadException("Failed to load music: " + filePath);
    else
        LOG_INFO("Loaded music: " + filePath);
}

void ResourceManager::playSound(const std::string &filePath) {
    auto it = soundBuffers.find(filePath);
    if (it == soundBuffers.end()) {
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(filePath))
            throw AudioLoadException("Failed to load sound: " + filePath);
        auto [newIt, success] =
            soundBuffers.emplace(filePath, std::move(buffer));
        it = newIt;
    }

    ResourceManager::updateSounds();

    if (activeSounds.size() >= MAX_CONCURRENT_SOUNDS) {
        for (auto &sound : activeSounds) {
            if (sound->getStatus() == sf::Sound::Stopped) {
                sound->setBuffer(it->second);
                sound->play();
                return;
            }
        }
        // Skip
        return;
    }

    auto newSound = std::make_unique<sf::Sound>();
    newSound->setBuffer(it->second);
    newSound->play();
    activeSounds.push_back(std::move(newSound));
}

void ResourceManager::updateSounds() {
    activeSounds.erase(std::remove_if(activeSounds.begin(), activeSounds.end(),
                                      [](const auto &sound) {
                                          return sound->getStatus() ==
                                                 sf::Sound::Stopped;
                                      }),
                       activeSounds.end());
}
