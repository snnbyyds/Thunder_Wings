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
#include "Core/ResourceManager.hpp"
#include <SFML/Config.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window.hpp>

Menu::Menu()
    : window(sf::VideoMode(screen_w, screen_h), "Thunder Wings"),
      active(false) {
    backgroundSprite.setTexture(ResourceManager::getTexture(backgroundPath));
    backgroundSprite.setPosition(0.0f, 0.0f);

    ResourceManager::loadFont(fontPath);

    ResourceManager::loadBackgroundMusic(musicPath);
    ResourceManager::gameBackgroundMusic.setVolume(8.0f);
    ResourceManager::gameBackgroundMusic.setLoop(true);
    // ResourceManager::gameBackgroundMusic.play();

    titleText.setFont(ResourceManager::gameFont);
    titleText.setString("THUNDER WINGS");
    titleText.setCharacterSize(80);
    titleText.setFillColor(sf::Color(255, 215, 0));
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(
        screen_w / 2.0f - titleText.getGlobalBounds().width / 2, 100);

    startText.setFont(ResourceManager::gameFont);
    startText.setString("START");
    startText.setCharacterSize(50);
    startText.setFillColor(sf::Color::Red);
    startText.setPosition(
        screen_w / 2.0f - startText.getGlobalBounds().width / 2, 300);

    loadText.setFont(ResourceManager::gameFont);
    loadText.setString("LOAD PROGRESS");
    loadText.setCharacterSize(50);
    loadText.setFillColor(sf::Color::White);
    loadText.setPosition(
        screen_w / 2.0f - loadText.getGlobalBounds().width / 2,
        400.0f
    );

    exitText.setFont(ResourceManager::gameFont);
    exitText.setString("EXIT");
    exitText.setCharacterSize(50);
    exitText.setFillColor(sf::Color::White);
    exitText.setPosition(
        (float)screen_w / 2 - exitText.getGlobalBounds().width / 2, 500);

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
        float progress = elapsed / LOGO_DURATION;

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
                    currentOption = std::min(currentOption + 1, MENU_MAX_OPTION);
                    break;
                case sf::Keyboard::Up:
                    currentOption = std::max(currentOption - 1, MENU_MIN_OPTION);
                    break;
                case sf::Keyboard::PageDown:
                    currentOption = MENU_MAX_OPTION;
                    break;
                case sf::Keyboard::PageUp:
                    currentOption = MENU_MIN_OPTION;
                    break;
                case sf::Keyboard::Enter:
                    switch (currentOption) {
                        case MENU_OPTION_START:
                            start();
                            break;
                        case MENU_OPTION_LOAD:
                            load();
                            break;
                        case MENU_OPTION_EXIT:
                            exit();
                            break;
                    }
                    return;
                default:
                    break;
            }
            startText.setFillColor(currentOption == MENU_OPTION_START ? sf::Color::Red : sf::Color::White);
            loadText.setFillColor(currentOption == MENU_OPTION_LOAD ? sf::Color::Red : sf::Color::White);
            exitText.setFillColor(currentOption == MENU_OPTION_EXIT ? sf::Color::Red : sf::Color::White);
        }
    }
}

void Menu::render() {
    window.clear();
    window.draw(backgroundSprite);
    window.draw(titleText);
    window.draw(startText);
    window.draw(loadText);
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

void Menu::exit() {
    active = false;
    window.close();
}
