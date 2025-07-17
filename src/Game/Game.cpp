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
#include "Core/Macros.hpp"
#include "Core/RandomUtils.hpp"
#include "Core/ResourceManager.hpp"
#include <iomanip>
#include <sstream>

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

    bossHealthText.setFont(ResourceManager::gameFont);
    bossHealthText.setCharacterSize(80);
    bossHealthText.setFillColor(sf::Color::Magenta);
    bossHealthText.setPosition(Constants::SCREEN_WIDTH / 2.0f - 200.0f, 20.0f);

    instructionText.setFont(ResourceManager::gameFont);
    instructionText.setString("Use Arrow Keys to Move");
    instructionText.setCharacterSize(40);
    instructionText.setFillColor(sf::Color::Cyan);
    instructionText.setStyle(sf::Text::Bold);
    sf::FloatRect instructionTextBounds = instructionText.getLocalBounds();
    instructionText.setOrigin(instructionTextBounds.width / 2,
                              instructionTextBounds.height / 2);
    instructionText.setPosition(Constants::SCREEN_WIDTH / 2.0f,
                                Constants::SCREEN_HEIGHT - 60.0f);

    pauseText.setFont(ResourceManager::gameFont);
    pauseText.setString("PAUSED");
    pauseText.setCharacterSize(80);
    pauseText.setFillColor(sf::Color::Cyan);
    pauseText.setStyle(sf::Text::Bold);
    sf::FloatRect pauseTextBounds = pauseText.getLocalBounds();
    pauseText.setOrigin(pauseTextBounds.width / 2, pauseTextBounds.height / 2);
    pauseText.setPosition(Constants::SCREEN_WIDTH / 2.0f,
                          Constants::SCREEN_HEIGHT / 2.0f);

    std::fill(enemyCount.begin(), enemyCount.end(), 0);
}

void Game::run() {
    running = true;
    paused = false;
    deltaTimer.restart();

    sf::Event event;
    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                terminated = true;
                break;
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::P) {
                    paused = !paused;
                }
                // We may add handling for more events here.
            }
        }

        if (paused) {
            deltaTimer.restart();
            showingInstructions = true;
        } else {
            float deltaTime = deltaTimer.getElapsedTime();
            deltaTimer.restart();
            if (!update(deltaTime))
                break;
        }

        render();
    }
    running = false;
}

void Game::bringGifts() {
    if (!player.gifts.empty())
        return;
    if (giftTimer.hasElapsed(Constants::GIFT_SPAWN_INTERVAL)) {
        if (!RandomUtils::chooseWithProb(Constants::GIFT_SPAWN_PROBABILITY)) {
            giftTimer.restart();
            return;
        }
        int choice = rand() & 1;
        switch (choice) {
            case 0:
                player.gifts.push_back(std::make_unique<FullFirePower>());
                break;
            case 1:
                player.gifts.push_back(std::make_unique<CenturyShield>());
                break;
            default: __unreachable(); break;
        }
        giftTimer.restart();
    }
}

void Game::spawnEnemies() {
    static const std::vector<int> levelSet = {1, 2, 3};
    static const std::vector<float> levelProb = {Constants::ENEMY1_SPAWN_PROB,
                                                 Constants::ENEMY2_SPAWN_PROB,
                                                 Constants::ENEMY3_SPAWN_PROB};
    float spawnInterval = RandomUtils::generateInRange(0.3f, 2.4f);
    if (spawnTimer.hasElapsed(spawnInterval)) {
        float spawnPositionX =
            RandomUtils::generateInRange(0.4f, Constants::SCREEN_WIDTH - 0.4f);
        int enemyLevel =
            RandomUtils::generateFromSetWithProb(levelSet, levelProb);
        switch (enemyLevel) {
            case 1:
                if (enemyCount[1] < Constants::ENEMY1_MAX_ALIVE) {
                    enemies.push_back(std::make_unique<Enemy1>(
                        sf::Vector2f(rand() % Constants::SCREEN_WIDTH, 0)));
                    enemyCount[1]++;
                }
                break;
            case 2:
                if (enemyCount[2] < Constants::ENEMY2_MAX_ALIVE) {
                    enemies.push_back(std::make_unique<Enemy2>(
                        sf::Vector2f(rand() % Constants::SCREEN_WIDTH, 0)));
                    enemyCount[2]++;
                }
                break;
            case 3:
                if (enemyCount[3] < Constants::ENEMY3_MAX_ALIVE &&
                    timeElapsed > 32.0f) {
                    // Spawn 32 enemy1
                    for (int i = 0; i < 32; i++)
                        enemies.push_back(std::make_unique<Enemy1>(
                            sf::Vector2f(rand() % Constants::SCREEN_WIDTH, 0)));

                    // Spawn 24 enemy2
                    for (int i = 0; i < 24; i++)
                        enemies.push_back(std::make_unique<Enemy2>(
                            sf::Vector2f(rand() % Constants::SCREEN_WIDTH, 0)));

                    // Spawn 1 enemy3
                    enemies.push_back(std::make_unique<Enemy3>(
                        sf::Vector2f(Constants::SCREEN_WIDTH / 2.0f, 0)));
                    enemyCount[1] += 32;
                    enemyCount[2] += 24;
                    enemyCount[3] += 1;
                }
                break;
            default: __unreachable(); break;
        }
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
    player.updateCollisions(bullets);

    std::ostringstream oss;

    if (!paused) {
        timeElapsed += deltaTime;
        oss << "Time: " << std::fixed << std::setprecision(3) << timeElapsed
            << 's';
        stopwatchText.setString(oss.str());
    }

    showingInstructions = (timeElapsed <= 8.0f);

    oss.clear();
    oss.str("");
    oss << "Health: " << std::fixed << std::setprecision(2) << player.health;
    healthText.setString(oss.str());

    if (player.health > 0.0f)
        player.shoot(bullets);

    // Update bullets
    for (auto it = bullets.begin(); it != bullets.end();) {
        if ((*it)->isAvailable()) {
            (*it)->update(deltaTime, player.getPosition());
            ++it;
        } else {
            if (!(*it)->exploding)
                bullets.erase(it);
            else
                ++it;
        }
    }

    // Update gifts
    for (auto it = player.gifts.begin(); it != player.gifts.end();) {
        if ((*it)->isAvailable()) {
            (*it)->update(deltaTime);
            ++it;
        } else {
            player.gifts.erase(it);
        }
    }

    // Update enemies
    currentBoss = nullptr;
    for (auto it = enemies.begin(); it != enemies.end();) {
        if ((*it)->isAvailable()) {
            (*it)->update(deltaTime);
            (*it)->shoot(bullets);
            (*it)->updateBulletCollisions(bullets);
            if ((*it)->level == 3 && (*it)->health > 0.0f)
                currentBoss = it->get();
            ++it;
        } else {
            int level = (*it)->level;
            if ((*it)->health <= 0.0f)
                player.health += (*it)->killBonus;
            enemyCount[level] = std::max(0, enemyCount[level] - 1);
            enemies.erase(it);
        }
    }
    if (currentBoss) {
        oss.clear();
        oss.str("");
        oss << "Boss: " << (int)currentBoss->health << std::endl
            << std::fixed << std::setprecision(5)
            << currentBoss->health / currentBoss->maxHealth * 100 << '%';
        bossHealthText.setString(oss.str());
    }

    spawnEnemies();
    bringGifts();
    return true;
}

void Game::render() {
    window.clear();
    window.draw(backgroundSprite);
    player.render(window);

    for (auto &bullet : bullets)
        bullet->render(window);

    for (auto &enemy : enemies)
        if (enemy->isAvailable())
            enemy->render(window);

    window.draw(stopwatchText);
    window.draw(healthText);
    if (currentBoss)
        window.draw(bossHealthText);
    drawGifts();

    if (paused) {
        sf::RectangleShape overlay(
            sf::Vector2f(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);
        window.draw(pauseText);
    }

    if (showingInstructions)
        window.draw(instructionText);

    window.display();
}

void Game::drawGifts() {
    if (player.gifts.empty())
        return;

    auto &gift = player.gifts.front();

    constexpr float padding = 10.0f;
    constexpr float iconSize = 64.0f;
    constexpr float raiseUp = 32.0f;
    float x = padding;
    float y = Constants::SCREEN_HEIGHT - iconSize - padding - raiseUp;

    auto icon = gift->getSprite();
    const auto bounds = icon.getLocalBounds();
    float scale = iconSize / bounds.height;
    gift->render(window);

    float remaining = gift->getRemainingTime();
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << remaining << "s";

    sf::Text text;
    text.setFont(ResourceManager::gameFont);
    text.setString(oss.str());
    text.setCharacterSize(34);
    if (remaining < 2.6f)
        text.setFillColor(sf::Color::Red);
    else
        text.setFillColor(sf::Color(4, 133, 255, 255));

    float textWidth = text.getLocalBounds().width;
    float iconW = icon.getGlobalBounds().width;
    text.setPosition(x + (iconW - textWidth) / 2.0f,
                     y + icon.getGlobalBounds().height + 4.0f);
    window.draw(text);
}
