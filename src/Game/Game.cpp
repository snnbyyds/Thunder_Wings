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

#include "Game/Game.hpp"
#include "Core/Constants.hpp"
#include "Core/ResourceManager.hpp"
#include <cstdlib>

Game::Game(sf::RenderWindow &window)
    : window(window), terminated(false), running(false) {
    backgroundSprite.setTexture(ResourceManager::getTexture(backgroundPath));
    backgroundSprite.setPosition(0.0f, 0.0f);

    stopwatchText.setFont(ResourceManager::gameFont);
    stopwatchText.setCharacterSize(30);
    stopwatchText.setFillColor(sf::Color::Red);
    stopwatchText.setPosition(Constants::SCREEN_WIDTH - 200.0f, 10.0f);

    healthText.setFont(ResourceManager::gameFont);
    healthText.setCharacterSize(30);
    healthText.setFillColor(sf::Color::Yellow);
    healthText.setPosition(Constants::SCREEN_WIDTH - 200.0f, 40.0f);

    srand((unsigned int)time(NULL));
}

void Game::run() {
    running = true;
    deltaTimer.restart();
    globalTimer.restart();

    sf::Event event;
    while (window.isOpen()) {
        float deltaTime = deltaTimer.getElapsedTime();
        deltaTimer.restart();

        if (!update(deltaTime))
            break;

        render();

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                terminated = true;
                break;
            }
        }
    }
    running = false;
}

void Game::spawnEnemies(float deltaTime) {
    float spawnInterval = (float)(rand() % 2) + 0.64f;
    if (spawnTimer.hasElapsed(spawnInterval)) {
        enemies.push_back(
            Enemy1(sf::Vector2f(rand() % Constants::SCREEN_WIDTH, 0)));
        spawnTimer.restart();
    }
}

bool Game::isRunning() { return running; }

bool Game::update(float deltaTime) {
    if (!player.isAvailable()) {
        gameOverText.setFont(ResourceManager::gameFont);
        gameOverText.setString("GAME OVER");
        gameOverText.setCharacterSize(80);
        gameOverText.setFillColor(sf::Color::Red);
        gameOverText.setPosition((float)Constants::SCREEN_WIDTH / 2 -
                                     gameOverText.getGlobalBounds().width / 2,
                                 (float)Constants::SCREEN_HEIGHT / 2 -
                                     gameOverText.getGlobalBounds().height / 2);
        window.draw(gameOverText);
        window.display();

        Timer waitTimer;
        while (!waitTimer.hasElapsed(3.2f))
            ;
        return false;
    }

    player.update(deltaTime);
    player.updateCollisions(bullets, enemies);

    float elapsedTime = globalTimer.getElapsedTime();
    stopwatchText.setString(
        "Time: " +
        std::to_string(elapsedTime)
            .substr(0, std::to_string(elapsedTime).find(".") + 3) +
        "s");

    healthText.setString(
        "Health: " +
        std::to_string(player.health)
            .substr(0, std::to_string(player.health).find(".") + 3));

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        player.shoot(bullets);

    // Update bullets
    for (auto it = bullets.begin(); it != bullets.end();) {
        if (it->isAvailable()) {
            it->update(deltaTime);
            ++it;
        } else {
            bullets.erase(it);
        }
    }

    // Update enemies
    for (auto it = enemies.begin(); it != enemies.end();) {
        if (it->isAvailable()) {
            it->update(deltaTime);
            it->shoot(bullets);
            it->updateBulletCollisions(bullets);
            ++it;
        } else {
            if (it->health <= 0.0f)
                player.health += it->level * 128.0f;
            enemies.erase(it);
        }
    }

    spawnEnemies(deltaTime);
    return true;
}

void Game::render() {
    window.clear();
    window.draw(backgroundSprite);
    player.render(window);

    for (auto &bullet : bullets)
        if (bullet.isAvailable())
            bullet.render(window);

    for (auto &enemy : enemies)
        if (enemy.isAvailable())
            enemy.render(window);

    window.draw(stopwatchText);
    window.draw(healthText);
    window.display();
}