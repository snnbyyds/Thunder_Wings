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
#include "Core/ResourceManager.hpp"
#include <SFML/Window.hpp>

Menu::Menu()
    : window(sf::VideoMode(screen_w, screen_h), "Thunder Wings"),
      active(false) {
    backgroundSprite.setTexture(ResourceManager::getTexture(backgroundPath));
    backgroundSprite.setPosition(0.0f, 0.0f);

    ResourceManager::loadFont(fontPath);

    ResourceManager::loadBackgroundMusic(musicPath);
    ResourceManager::gameBackgroundMusic.setLoop(true);
    ResourceManager::gameBackgroundMusic.play();

    titleText.setFont(ResourceManager::gameFont);
    titleText.setString("THUNDER WINGS");
    titleText.setCharacterSize(80);
    titleText.setFillColor(sf::Color(255, 215, 0));
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(
        (float)screen_w / 2 - titleText.getGlobalBounds().width / 2, 100);

    startText.setFont(ResourceManager::gameFont);
    startText.setString("START");
    startText.setCharacterSize(50);
    startText.setFillColor(sf::Color::Red);
    startText.setPosition(
        (float)screen_w / 2 - startText.getGlobalBounds().width / 2, 300);

    exitText.setFont(ResourceManager::gameFont);
    exitText.setString("EXIT");
    exitText.setCharacterSize(50);
    exitText.setFillColor(sf::Color::White);
    exitText.setPosition(
        (float)screen_w / 2 - exitText.getGlobalBounds().width / 2, 380);
}

void Menu::show() {
    active = true;
    if (game != nullptr && (!game->isRunning() || game->terminated)) {
        delete game;
        game = nullptr;
    }

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

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Up) {
                startText.setFillColor(sf::Color::Red);
                exitText.setFillColor(sf::Color::White);
            } else if (event.key.code == sf::Keyboard::Down) {
                startText.setFillColor(sf::Color::White);
                exitText.setFillColor(sf::Color::Red);
            } else if (event.key.code == sf::Keyboard::Enter) {
                if (startText.getFillColor() == sf::Color::Red)
                    start();
                else if (exitText.getFillColor() == sf::Color::Red)
                    exit();
            }
        }
    }
}

void Menu::render() {
    window.clear();
    window.draw(backgroundSprite);
    window.draw(titleText);
    window.draw(startText);
    window.draw(exitText);
    window.display();
}

void Menu::start() {
    active = false;
    game = new Game(window);
    game->run();
    if (game->terminated)
        exit();
    else
        show();
}

void Menu::exit() {
    active = false;
    window.close();
}