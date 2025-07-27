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

#include "Game/Menu.hpp"
#include "Core/Constants.hpp"
#include "Core/Macros.h"
#include "Core/ResourceManager.hpp"
#include <SFML/Config.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window.hpp>

Menu::Menu()
    : window(sf::VideoMode(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT),
             "Thunder Wings"),
      active(false) {
    backgroundSprite.setTexture(
        ResourceManager::getTexture(Constants::BACKGROUND_FILE_NAME));
    backgroundSprite.setPosition(0.0f, 0.0f);

    ResourceManager::loadGameFont(Constants::GAME_FONT);
    ResourceManager::loadPageFont(Constants::PARAGRAPH_FONT);

    ResourceManager::loadBackgroundMusic(Constants::BGM_FILE_NAME);
    ResourceManager::gameBackgroundMusic.setVolume(8.0f);
    ResourceManager::gameBackgroundMusic.setLoop(true);
    // ResourceManager::gameBackgroundMusic.play();

    titleText.setFont(ResourceManager::gameFont);
    titleText.setString("THUNDER WINGS");
    titleText.setCharacterSize(80);
    titleText.setFillColor(sf::Color(255, 215, 0));
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(Constants::SCREEN_WIDTH / 2.0f -
                              titleText.getGlobalBounds().width / 2,
                          100.0f);

    startText.setFont(ResourceManager::gameFont);
    startText.setString("START");
    startText.setCharacterSize(50);
    startText.setFillColor(sf::Color::Green);
    startText.setPosition(Constants::SCREEN_WIDTH / 2.0f -
                              startText.getGlobalBounds().width / 2,
                          300.0f);

    loadText.setFont(ResourceManager::gameFont);
    loadText.setString("LOAD PROGRESS");
    loadText.setCharacterSize(50);
    loadText.setFillColor(sf::Color::White);
    loadText.setPosition(Constants::SCREEN_WIDTH / 2.0f -
                             loadText.getGlobalBounds().width / 2,
                         400.0f);

    guideText.setFont(ResourceManager::gameFont);
    guideText.setString("GUIDE");
    guideText.setCharacterSize(50);
    guideText.setFillColor(sf::Color::White);
    guideText.setPosition(Constants::SCREEN_WIDTH / 2.0f -
                              guideText.getGlobalBounds().width / 2,
                          500.0f);

    aboutText.setFont(ResourceManager::gameFont);
    aboutText.setString("ABOUT");
    aboutText.setCharacterSize(50);
    aboutText.setFillColor(sf::Color::White);
    aboutText.setPosition(Constants::SCREEN_WIDTH / 2.0f -
                              guideText.getGlobalBounds().width / 2,
                          600.0f);

    exitText.setFont(ResourceManager::gameFont);
    exitText.setString("EXIT");
    exitText.setCharacterSize(50);
    exitText.setFillColor(sf::Color::White);
    exitText.setPosition(Constants::SCREEN_WIDTH / 2.0f -
                             exitText.getGlobalBounds().width / 2,
                         700.0f);

    sf::Texture &logoTexture =
        ResourceManager::getTexture("assets/mujianwu.png");
    logoTexture.setSmooth(true);
    logoSprite.setTexture(logoTexture);
    auto [x, y] = logoTexture.getSize();
    logoSprite.setOrigin(x >> 1, y >> 1);
    logoSprite.setPosition(Constants::SCREEN_WIDTH / 2.0f,
                           Constants::SCREEN_HEIGHT / 2.0f);
    logoSprite.setColor(sf::Color(255, 255, 255, 0));
}

Menu::~Menu() { game.reset(); }

void Menu::playLogo() {
    logoClock.restart();
    showingLogo = true;

    sf::Event event;
    while (showingLogo && window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                terminated = true;
                exit();
                return;
            }
        }

        float elapsed = logoClock.getElapsedTime();
        float progress = elapsed / Constants::LOGO_DURATION;

        if (progress < 1.0f) {
            float alpha;
            if (progress < 0.3f)
                alpha = 255 * (progress / 0.3f);
            else if (progress < 0.7f)
                alpha = 255;
            else
                alpha = 255 * (1.0f - (progress - 0.7f) / 0.3f);
            logoSprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));

            window.clear(sf::Color::Black);
            window.draw(logoSprite);
            window.display();
        } else {
            showingLogo = false;
        }
    }
}

void Menu::show() {
    if (terminated)
        return;

    if (!musicStarted) {
        ResourceManager::gameBackgroundMusic.play();
        musicStarted = true;
    }

    active = true;
    if (game != nullptr && (!game->isRunning() || game->terminated))
        game.reset();

    while (active) {
        handleInput();
        render();
    }
}

void Menu::handleInput() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            exit();
        else if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Down:
                    currentOption =
                        std::min(currentOption + 1, MENU_MAX_OPTION);
                    break;
                case sf::Keyboard::Up:
                    currentOption =
                        std::max(currentOption - 1, MENU_MIN_OPTION);
                    break;
                case sf::Keyboard::PageDown:
                    currentOption = MENU_MAX_OPTION;
                    break;
                case sf::Keyboard::PageUp:
                    currentOption = MENU_MIN_OPTION;
                    break;
                case sf::Keyboard::Enter:
                    switch (currentOption) {
                        case MENU_OPTION_START: start(); break;
                        case MENU_OPTION_LOAD: load(); break;
                        case MENU_OPTION_GUIDE: showGuide(); break;
                        case MENU_OPTION_ABOUT: showAbout(); break;
                        case MENU_OPTION_EXIT: exit(); break;
                    }
                    return;
                default: break;
            }
            startText.setFillColor(currentOption == MENU_OPTION_START
                                       ? sf::Color::Green
                                       : sf::Color::White);
            loadText.setFillColor(currentOption == MENU_OPTION_LOAD
                                      ? sf::Color::Yellow
                                      : sf::Color::White);
            guideText.setFillColor(currentOption == MENU_OPTION_GUIDE
                                       ? sf::Color::Yellow
                                       : sf::Color::White);
            aboutText.setFillColor(currentOption == MENU_OPTION_ABOUT
                                       ? sf::Color::Yellow
                                       : sf::Color::White);
            exitText.setFillColor(currentOption == MENU_OPTION_EXIT
                                      ? sf::Color::Red
                                      : sf::Color::White);
        }
    }
}

void Menu::render() {
    window.clear();
    window.draw(backgroundSprite);
    window.draw(titleText);
    window.draw(startText);
    window.draw(loadText);
    window.draw(guideText);
    window.draw(aboutText);
    window.draw(exitText);
    window.display();
}

void Menu::start() {
    active = false;
    game = std::make_unique<Game>(window);
    game->run();
    if (game->terminated)
        exit();
    else
        show();
}

void Menu::load() {
    game = std::make_unique<Game>(window);
    game->loadFromDisk();
    game->run();
    active = false;
    if (game->terminated)
        exit();
    else
        show();
}

// clang-format off
const std::vector<std::string> Menu::guideLines = {
    "-- CONTROLS --",
    "Move: Arrow Keys     Pause: P     Menu: Esc",
    "",
    "-- PLAYER --",
    "Fire: 2 bullets @ 6144 dmg every 0.144s",
    "Heal: +128 HP every 0.64s",
    "Lifesteal: +50% enemy HP on normal kill",
    "           +100% boss HP on boss kill",
    "Max HP: 1,024,000",
    "",
    "-- ENEMIES --",
    "Enemy I    HP 2048   Speed 128-256   Dmg 512",
    "Enemy II   HP 16,384 Speed 64-256    Dmg 728 (sine-wave motion)",
    "Boss III   HP 840k-1200k  Circles top of screen",
    "           Heal: +2100 HP every 1.0s",
    "           Volley: 6 bullets (6*4096)",
    "           Missiles: 17,203 dmg each",
    "           Rockets: 6,553.6 dmg min or up to 18.4% of your current HP",
    "",
    "-- GIFTS (every 32s, 48% chance) --",
    "FullFirePower: +400% fire rate",
    "CenturyShield: 90% damage reduction",
    "AllMyPeople: Make your enemies fight for you",
    "",
    "Press Esc to return to menu"
};
// clang-format on

void Menu::showGuide() { displayText(guideLines); }

void Menu::exit() {
    active = false;
    window.close();
}

// clang-format off
const std::vector<std::string> Menu::aboutLines = {
    "-- ABOUT --",
    "Thunder_Wings " TW_VERSION, // NOLINT(bugprone-suspicious-missing-comma)
    "",
    "-- BUILD INFO --",
    "Compiler: " TW_COMPILER_OPTIONS, // NOLINT(bugprone-suspicious-missing-comma)
    "Linker: " TW_LINKER_OPTIONS, // NOLINT(bugprone-suspicious-missing-comma)
    "",
    "-- LEGAL --",
    TW_COPYRIGHT,
    "Licensed under " TW_LICENSE, // NOLINT(bugprone-suspicious-missing-comma)
    "",
    "-- ACKNOWLEDGMENTS --",
    "Built with SFML (Simple and Fast Multimedia Library)",
    "Built with Boost C++ Libraries",
    "Special thanks to the open source community",
    "",
    "-- AUTHOR --",
    "Nuo Shen",
    "",
    "Press Esc to return to menu"
};
// clang-format on

void Menu::showAbout() { displayText(aboutLines); }

void Menu::displayText(const std::vector<std::string> &lines) {
    std::vector<sf::Text> textItems;
    float y = 8.f;
    for (auto &str : lines) {
        sf::Text t;
        t.setFont(ResourceManager::pageFont);
        if (str.rfind("--", 0) == 0) {
            // Section header
            t.setCharacterSize(28);
            t.setFillColor(sf::Color::Red);
            y += 4.f;
        } else {
            t.setCharacterSize(24);
            t.setFillColor(sf::Color::Black);
        }
        t.setString(str);
        t.setPosition(50.f, y);
        textItems.push_back(t);
        y += t.getCharacterSize() + (str.rfind("--", 0) == 0 ? 14.f : 10.f);
    }

    sf::Event event;
    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape) {
                return;
            }
            if (event.type == sf::Event::Closed) {
                exit();
                return;
            }
        }
        window.clear();
        window.draw(backgroundSprite);
        for (auto &t : textItems)
            window.draw(t);
        window.display();
    }
}
