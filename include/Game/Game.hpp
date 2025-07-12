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
#include "../Core/Timer.hpp"
#include "../Entities/Bullet.hpp"
#include "../Entities/Enemy.hpp"
#include "../Entities/Player.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class Game {
public:
    Game(sf::RenderWindow &window);
    void run();
    void spawnEnemies(float deltaTime);
    bool isRunning();

    bool terminated;

private:
    bool update(float deltaTime);
    void render();

    const char *backgroundPath = "assets/background.png";
    sf::RenderWindow &window;
    sf::Sprite backgroundSprite;
    Timer globalTimer;
    Timer deltaTimer;
    Timer spawnTimer;
    sf::Text gameOverText;
    sf::Text stopwatchText;
    sf::Text healthText;
    std::vector<Bullet> bullets;
    std::vector<std::unique_ptr<Enemy>> enemies;
    Player player;
    bool running;
};