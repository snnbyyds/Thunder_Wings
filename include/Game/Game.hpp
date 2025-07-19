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
#include "../Core/Constants.hpp"
#include "../Core/ISerializable.hpp"
#include "../Core/Timer.hpp"
#include "../Entities/Bullet.hpp"
#include "../Entities/Enemy.hpp"
#include "../Entities/Player.hpp"
#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include <vector>

// clang-format off
#define PAUSE_OPTION_RESUME 0
#define PAUSE_OPTION_SAVE   1
#define PAUSE_OPTION_EXIT   2
#define PAUSE_MAX_OPTION    2
#define PAUSE_MIN_OPTION    0
// clang-format on

class Game : public ISerializable {
public:
    Game(sf::RenderWindow &window);

    void run();
    void bringGifts();
    void spawnEnemies();
    bool isRunning();

    boost::json::object serialize() const override;
    void deserialize(const boost::json::object &o) override;

    void loadFromDisk();
    void saveToDisk();

    bool terminated;

private:
    bool update(float deltaTime);
    void render();
    void drawGifts();

    sf::RenderWindow &window;

    const char *backgroundPath = "assets/background.png";
    sf::Sprite backgroundSprite;

    sf::Text gameOverText;
    sf::Text stopwatchText;
    sf::Text healthText;
    sf::Text killedText;
    sf::Text bossHealthText;
    sf::Text instructionText;

    sf::Text pauseText;
    sf::Text resumeText;
    sf::Text saveText;
    sf::Text exitText;

    Timer deltaTimer;
    Timer giftTimer;
    Timer spawnTimer;
    float timeElapsed = 0.0f;

    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::array<int, Constants::ENEMY_LEVEL_COUNT + 1ul> enemyCount;
    Player player;
    Enemy *currentBoss = nullptr;

    bool running = false;
    bool showingInstructions = false;

    bool paused = false;
    int currentPauseOption = PAUSE_OPTION_RESUME;

    size_t killed = 0ul;
};