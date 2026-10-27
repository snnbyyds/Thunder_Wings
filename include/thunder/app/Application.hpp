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

#include "thunder/assets/AssetManager.hpp"
#include "thunder/game/GameWorld.hpp"
#include "thunder/persistence/SaveRepository.hpp"
#include "thunder/render/GameRenderer.hpp"
#include "thunder/render/MenuRenderer.hpp"

#include <SFML/Graphics.hpp>

#include <filesystem>
#include <optional>
#include <span>
#include <string>

namespace thunder {

struct ApplicationOptions {
    std::filesystem::path executablePath;
    std::optional<std::filesystem::path> assetsDirectory;
    std::optional<std::filesystem::path> saveFile;
    bool audioEnabled{true};
    bool smokeTest{false};
};

class Application {
public:
    explicit Application(ApplicationOptions options);

    Application(const Application &) = delete;
    Application &operator=(const Application &) = delete;

    int run();

private:
    enum class Screen {
        Menu,
        Guide,
        About,
        Playing,
        Paused,
        GameOver,
    };

    static constexpr int kMenuOptionCount = 5;
    static constexpr int kPauseOptionCount = 3;

    void processEvents();
    void handleKey(sf::Keyboard::Key key);
    void update(float deltaTime);
    void render();

    void startNewGame();
    void loadGame();
    void saveGame();
    void enterMenu();
    void pauseGame();
    void resumeGame();
    void drainSounds();

    [[nodiscard]] PlayerInput currentInput() const;
    [[nodiscard]] std::span<const std::string> guideLines() const;
    [[nodiscard]] std::span<const std::string> aboutLines() const;
    void runSmokeTest();

    ApplicationOptions options_;
    std::filesystem::path assetsDirectory_;
    sf::RenderWindow window_;
    AssetManager assets_;
    GameRenderer gameRenderer_;
    MenuRenderer menuRenderer_;
    std::optional<SaveRepository> saveRepository_;
    std::optional<GameWorld> world_;

    Screen screen_{Screen::Menu};
    int menuSelection_{};
    int pauseSelection_{};
    float accumulator_{};
    float gameOverTimer_{};
    std::string menuMessage_;
    std::string pauseMessage_;
};

} // namespace thunder
