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
#include "Core/Logging.hpp"
#include "Core/Macros.h"
#include "Core/RandomUtils.hpp"
#include "Core/ResourceManager.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

Game::Game(sf::RenderWindow &window)
    : terminated(false), window(window), running(false) {
    backgroundSprite.setTexture(
        ResourceManager::getTexture(Constants::BACKGROUND_FILE_NAME));
    backgroundSprite.setPosition(0.0f, 0.0f);

    stopwatchText.setFont(ResourceManager::gameFont);
    stopwatchText.setCharacterSize(30);
    stopwatchText.setFillColor(sf::Color::Red);
    stopwatchText.setPosition(Constants::SCREEN_WIDTH - 200.0f, 10.0f);

    healthText.setFont(ResourceManager::gameFont);
    healthText.setCharacterSize(30);
    healthText.setFillColor(sf::Color::Yellow);
    healthText.setPosition(Constants::SCREEN_WIDTH - 200.0f, 40.0f);

    killedText.setFont(ResourceManager::gameFont);
    killedText.setCharacterSize(32);
    killedText.setFillColor(sf::Color::Green);
    killedText.setPosition(Constants::SCREEN_WIDTH - 200.0f, 80.0f);

    bossHealthText.setFont(ResourceManager::gameFont);
    bossHealthText.setCharacterSize(80);
    bossHealthText.setFillColor(sf::Color::Magenta);
    bossHealthText.setPosition(Constants::SCREEN_WIDTH / 2.0f - 200.0f, 20.0f);

    instructionText.setFont(ResourceManager::gameFont);
    instructionText.setString("Use Arrow Keys to Move");
    instructionText.setCharacterSize(40);
    instructionText.setFillColor(sf::Color::Green);
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
                          Constants::SCREEN_HEIGHT / 2.0f - 350);

    resumeText.setFont(ResourceManager::gameFont);
    resumeText.setCharacterSize(60);
    resumeText.setString("Resume");
    sf::FloatRect resumeTextBounds = resumeText.getLocalBounds();
    resumeText.setOrigin(resumeTextBounds.width / 2,
                         resumeTextBounds.height / 2);
    resumeText.setPosition(Constants::SCREEN_WIDTH / 2.0f,
                           pauseText.getPosition().y + 260);

    saveText.setFont(ResourceManager::gameFont);
    saveText.setCharacterSize(60);
    saveText.setString("Save Progress");
    sf::FloatRect saveTextBounds = saveText.getLocalBounds();
    saveText.setOrigin(saveTextBounds.width / 2, saveTextBounds.height / 2);
    saveText.setPosition(Constants::SCREEN_WIDTH / 2.0f,
                         resumeText.getPosition().y + 80);

    exitText.setFont(ResourceManager::gameFont);
    exitText.setCharacterSize(60);
    exitText.setString("Exit");
    sf::FloatRect exitTextBounds = exitText.getLocalBounds();
    exitText.setOrigin(exitTextBounds.width / 2, exitTextBounds.height / 2);
    exitText.setPosition(Constants::SCREEN_WIDTH / 2.0f,
                         saveText.getPosition().y + 80);

    std::fill(enemyCount.begin(), enemyCount.end(), 0);

    // Determine file for saved progress
    static char path[SAVE_PATH_MAX];
    if (get_save_file_path(path))
        throw std::runtime_error("Cannot determine save path");
    save_file = path;
}

void Game::run() {
    running = true;
    paused = false;
    deltaTimer.restart();

    sf::Event event;
    while (running && window.isOpen()) {
        while (running && window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                terminated = true;
                break;
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::P) {
                    paused = !paused;
                    currentPauseOption = PAUSE_OPTION_RESUME;
                    deltaTimer.restart();
                } else if (event.key.code == sf::Keyboard::Escape) {
                    running = false;
                    break;
                } else if (paused) {
                    switch (event.key.code) {
                        case sf::Keyboard::Down:
                            currentPauseOption = std::min(
                                currentPauseOption + 1, PAUSE_MAX_OPTION);
                            break;
                        case sf::Keyboard::Up:
                            currentPauseOption = std::max(
                                currentPauseOption - 1, PAUSE_MIN_OPTION);
                            break;
                        case sf::Keyboard::PageDown:
                            currentPauseOption = PAUSE_MAX_OPTION;
                            break;
                        case sf::Keyboard::PageUp:
                            currentPauseOption = PAUSE_MIN_OPTION;
                            break;
                        case sf::Keyboard::Enter:
                            switch (currentPauseOption) {
                                case PAUSE_OPTION_RESUME:
                                    paused = false;
                                    deltaTimer.restart();
                                    break;
                                case PAUSE_OPTION_SAVE: saveToDisk(); break;
                                case PAUSE_OPTION_EXIT: running = false; break;
                            }
                            break;
                        default: break;
                    }
                }
            }
        }

        if (!running)
            break;

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
    if (player.gifts.size() >= 3ul)
        return;

    if (giftTimer.hasElapsed(Constants::GIFT_SPAWN_INTERVAL) &&
        RandomUtils::chooseWithProb(Constants::GIFT_SPAWN_PROBABILITY)) {
        int count = RandomUtils::chooseWithProb(
                        Constants::GIFT_SPAWN_PROBABILITY / 2.0f)
                        ? (currentBoss ? 3 : 2)
                        : 1;
        std::vector<int> choices = {0, 1, 2, 3};
        std::vector<float> ratios = {20.0f, 25.0f, 15.0f, 40.0f};
        for (int i = 0; i < count; i++) {
            int choice = RandomUtils::generateFromSetWithRatio(choices, ratios);
            switch (choice) {
                case 0:
                    player.gifts.push_back(std::make_unique<FullFirePower>());
                    break;
                case 1:
                    player.gifts.push_back(std::make_unique<CenturyShield>());
                    break;
                case 2:
                    player.gifts.push_back(std::make_unique<AllMyPeople>());
                    break;
                case 3:
                    player.gifts.push_back(std::make_unique<SpeedStorm>());
                    break;
                default: __unreachable(); break;
            }
            for (int i = 0; i < choices.size(); i++)
                if (choice == choices[i])
                    ratios[i] = 0.0f;
        }
        giftTimer.restart();
    }
}

void Game::spawnEnemies() {
    static const std::vector<int> levelSet = {1, 2, 3};
    static const std::vector<float> levelProb = {Constants::ENEMY1_SPAWN_PROB,
                                                 Constants::ENEMY2_SPAWN_PROB,
                                                 Constants::ENEMY3_SPAWN_PROB};
    float spawnInterval = currentBoss ? 0.1f : 0.6f;
    if (spawnTimer.hasElapsed(spawnInterval)) {
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

boost::json::object Game::serialize() const {
    boost::json::object o = {{"deltaTime", deltaTimer.getElapsedTime()},
                             {"giftTime", giftTimer.getElapsedTime()},
                             {"spawnTime", spawnTimer.getElapsedTime()},
                             {"timeElapsed", timeElapsed},
                             {"killed", killed}};

    // bullets
    boost::json::array bulletsArray;
    for (const auto &bullet : bullets)
        bulletsArray.push_back(bullet->serialize());
    o["bullets"] = std::move(bulletsArray);

    // enemies
    boost::json::array enemiesArray;
    for (const auto &enemy : enemies)
        enemiesArray.push_back(enemy->serialize());
    o["enemies"] = std::move(enemiesArray);

    // enemyCount will be deduced from enemies

    // player
    o["player"] = player.serialize();

    // gifts
    boost::json::array giftsArray;
    for (const auto &gift : player.gifts)
        giftsArray.push_back(gift->serialize());
    o["gifts"] = std::move(giftsArray);

    return o;
}

void Game::deserialize(const boost::json::object &o) {
    deltaTimer.setElapsedTime((float)o.at("deltaTime").as_double());
    giftTimer.setElapsedTime((float)o.at("giftTime").as_double());
    spawnTimer.setElapsedTime((float)o.at("spawnTime").as_double());
    timeElapsed = (float)o.at("timeElapsed").as_double();
    killed = (size_t)o.at("killed").as_int64();

    // bullets
    bullets.clear();
    for (const auto &v : o.at("bullets").as_array()) {
        auto obj = v.as_object();
        bool avail = obj.at("avail").as_bool();
        if (!avail)
            continue;
        std::string type = obj.at("type").as_string().c_str();
        if (type == "Cannon")
            bullets.push_back(std::make_unique<Cannon>(obj));
        else if (type == "Missile")
            bullets.push_back(std::make_unique<Missile>(obj));
        else if (type == "Rocket")
            bullets.push_back(std::make_unique<Rocket>(obj));
        else
            LOG_WARN("Unrecognized bullet type: " << type);
    }

    // enemies
    std::fill(enemyCount.begin(), enemyCount.end(), 0);
    enemies.clear();
    for (const auto &v : o.at("enemies").as_array()) {
        auto obj = v.as_object();
        bool avail = obj.at("avail").as_bool();
        if (!avail)
            continue;
        int level = (int)obj.at("level").as_int64();
        switch (level) {
            case 1: enemies.push_back(std::make_unique<Enemy1>(obj)); break;
            case 2: enemies.push_back(std::make_unique<Enemy2>(obj)); break;
            case 3: enemies.push_back(std::make_unique<Enemy3>(obj)); break;
            default: LOG_WARN("Unrecognized enemy level: " << level); break;
        }
    }
    for (const auto &enemy : enemies)
        if (enemy->isAvailable() && !enemy->charmed)
            enemyCount[enemy->level]++;

    // player
    player.deserialize(o.at("player").as_object());

    // gifts
    for (const auto &v : o.at("gifts").as_array()) {
        auto obj = v.as_object();
        bool avail = obj.at("avail").as_bool();
        if (!avail)
            continue;
        std::string name = obj.at("name").as_string().c_str();
        if (name == "FullFirePower")
            player.gifts.push_back(std::make_unique<FullFirePower>(obj));
        else if (name == "CenturyShield")
            player.gifts.push_back(std::make_unique<CenturyShield>(obj));
        else if (name == "AllMyPeople")
            player.gifts.push_back(std::make_unique<AllMyPeople>(obj));
        else if (name == "SpeedStorm")
            player.gifts.push_back(std::make_unique<SpeedStorm>(obj));
        else
            LOG_WARN("Unrecognized gift name: " << name);
    }
}

void Game::loadFromDisk() {
    // Load file
    LOG_INFO("Loading progress from " << save_file);
    std::ifstream ifs(save_file, std::ios::binary);
    if (!ifs.is_open()) {
        LOG_ERROR("Load " << save_file << " failed!");
        return;
    }
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        std::istreambuf_iterator<char>());
    ifs.close();

    // Parse JSON
    try {
        deserialize(boost::json::parse(content).as_object());
    } catch (const boost::system::system_error &e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    }
}

void Game::saveToDisk() {
    LOG_INFO("Saving progress to disk");
    std::ofstream ofs(save_file, std::ios::trunc | std::ios::binary);
    if (!ofs.is_open())
        throw std::runtime_error("Cannot open file for writing: " +
                                 std::string(save_file));
    ofs << boost::json::serialize(serialize());
    ofs.close();
}

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

    oss.clear();
    oss.str("");
    oss << "killed: " << killed;
    killedText.setString(oss.str());

    if (player.health > 0.0f)
        player.shoot(bullets);

    // Update bullets
    sf::Vector2f hitTarget = player.getPosition();
    // Hit the strongest charmed enemy
    float maxCharmedHealth = 0.0f;
    for (const auto &enemy : enemies) {
        if (enemy->isAvailable() && enemy->charmed &&
            enemy->health > maxCharmedHealth) {
            hitTarget = enemy->getPosition();
            maxCharmedHealth = enemy->health;
        }
    }
    for (auto it = bullets.begin(); it != bullets.end();) {
        if ((*it)->isAvailable()) {
            (*it)->update(deltaTime, hitTarget);
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
        int level = (*it)->level;
        if ((*it)->isAvailable()) {
            (*it)->update(deltaTime);
            (*it)->shoot(bullets);
            (*it)->updateBulletCollisions(bullets);
            if (!(*it)->bonusTaken && (*it)->charmed) {
                player.health += (*it)->killBonus;
                killed++;
                enemyCount[level] = std::max(0, enemyCount[level] - 1);
                (*it)->bonusTaken = true;
            } else if ((*it)->level == 3 && (*it)->health > 0.0f)
                currentBoss = it->get();
            ++it;
        } else {
            if (!(*it)->bonusTaken && (*it)->health <= 0.0f) {
                player.health += (*it)->killBonus;
                killed++;
                (*it)->bonusTaken = true;
            }
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

    if (player.charming)
        backgroundSprite.setColor(sf::Color(244, 154, 240, 232));
    else
        backgroundSprite.setColor(sf::Color::White);
    window.draw(backgroundSprite);

    player.render(window);

    for (auto &bullet : bullets)
        bullet->render(window);

    for (auto &enemy : enemies)
        if (enemy->isAvailable())
            enemy->render(window);

    window.draw(stopwatchText);
    window.draw(healthText);
    window.draw(killedText);
    if (currentBoss)
        window.draw(bossHealthText);
    drawGifts();

    if (paused) {
        sf::RectangleShape overlay(
            sf::Vector2f(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);
        window.draw(pauseText);

        resumeText.setFillColor(currentPauseOption == PAUSE_OPTION_RESUME
                                    ? sf::Color::Blue
                                    : sf::Color::White);
        saveText.setFillColor(currentPauseOption == PAUSE_OPTION_SAVE
                                  ? sf::Color::Blue
                                  : sf::Color::White);
        exitText.setFillColor(currentPauseOption == PAUSE_OPTION_EXIT
                                  ? sf::Color::Red
                                  : sf::Color::White);
        window.draw(resumeText);
        window.draw(saveText);
        window.draw(exitText);
    }

    if (showingInstructions)
        window.draw(instructionText);

    window.display();
}

void Game::drawGifts() {
    if (player.gifts.empty())
        return;

    constexpr float padding = 20.0f;
    constexpr float iconSize = 80.0f;
    constexpr float spacing = 15.0f;
    constexpr float textGap = 8.0f;

    float startX = padding;
    float baseY = Constants::SCREEN_HEIGHT - iconSize - padding - 40;

    for (size_t i = 0; i < player.gifts.size(); ++i) {
        auto &gift = player.gifts[i];

        float x = startX + i * (iconSize + spacing);
        float y = baseY;
        sf::RectangleShape background;
        background.setSize(sf::Vector2f(iconSize + 8, iconSize + 40 + 8));
        background.setPosition(x - 4, y - 4);
        background.setFillColor(sf::Color(0, 0, 0, 120));
        background.setOutlineThickness(2.0f);
        background.setOutlineColor(sf::Color(255, 255, 255, 180));
        window.draw(background);

        // Icon
        sf::Sprite icon = gift->getSprite();
        icon.setPosition(x, y);
        window.draw(icon);

        // Timer
        float remaining = gift->getRemainingTime();
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << remaining << "s";

        sf::Text text;
        text.setFont(ResourceManager::gameFont);
        text.setString(oss.str());
        text.setCharacterSize(26);

        if (remaining < 2.0f) {
            text.setFillColor(sf::Color::Red);
        } else if (remaining < 5.0f) {
            text.setFillColor(sf::Color::Yellow);
        } else {
            text.setFillColor(sf::Color::Green);
        }

        sf::FloatRect textBounds = text.getLocalBounds();
        float textX = x + (iconSize - textBounds.width) / 2.0f;
        float textY = y + iconSize + textGap;
        text.setPosition(textX, textY);
        window.draw(text);

        // Flash
        if (remaining < 3.0f) {
            static sf::Clock blinkClock;
            float blinkTime = blinkClock.getElapsedTime().asSeconds();
            float blinkSpeed = (remaining < 1.0f) ? 4.0f : 2.0f;

            if (std::fmod(blinkTime * blinkSpeed, 2.0f) < 1.0f) {
                sf::RectangleShape alertOverlay;
                alertOverlay.setSize(sf::Vector2f(iconSize, iconSize));
                alertOverlay.setPosition(x, y);
                alertOverlay.setFillColor(sf::Color(255, 100, 100, 80));
                window.draw(alertOverlay);
            }
        }

        float maxTime = gift->maxTime;
        if (maxTime > 0) {
            float progress = remaining / maxTime;

            sf::RectangleShape progressBg;
            progressBg.setSize(sf::Vector2f(iconSize, 4));
            progressBg.setPosition(x, y + iconSize + 2);
            progressBg.setFillColor(sf::Color(50, 50, 50, 200));
            window.draw(progressBg);

            sf::RectangleShape progressBar;
            progressBar.setSize(sf::Vector2f(iconSize * progress, 4));
            progressBar.setPosition(x, y + iconSize + 2);

            if (progress < 0.2f) {
                progressBar.setFillColor(sf::Color::Red);
            } else if (progress < 0.5f) {
                progressBar.setFillColor(sf::Color::Yellow);
            } else {
                progressBar.setFillColor(sf::Color::Green);
            }
            window.draw(progressBar);
        }
    }
}
