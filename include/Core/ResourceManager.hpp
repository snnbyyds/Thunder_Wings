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
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

class FontLoadException : public std::runtime_error {
public:
    explicit FontLoadException(const std::string &message)
        : std::runtime_error(message) {}
};

class AudioLoadException : public std::runtime_error {
public:
    explicit AudioLoadException(const std::string &message)
        : std::runtime_error(message) {}
};

class TextureLoadException : public std::runtime_error {
public:
    explicit TextureLoadException(const std::string &message)
        : std::runtime_error(message) {}
};

class ResourceManager {
public:
    ResourceManager() = delete;

    static sf::Texture &getTexture(const std::string &texturePath);
    static bool getTextureifExists(const std::string &texturePath);
    static void loadGameFont(const std::string &fontPath);
    static void loadPageFont(const std::string &fontPath);
    static void loadBackgroundMusic(const std::string &filePath);
    static void playSound(const std::string &filePath);
    static void updateSounds();

    static sf::Font gameFont;
    static sf::Font pageFont;
    static sf::Music gameBackgroundMusic;

private:
    static std::unordered_map<std::string, sf::Texture> textures;
    static std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
    static std::vector<std::unique_ptr<sf::Sound>> activeSounds;

    static constexpr size_t MAX_CONCURRENT_SOUNDS = 16;
};