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
#include "Game.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class Menu {
public:
    Menu();
    void show();
    void start();
    void exit();

    static constexpr int screen_w = 1440;
    static constexpr int screen_h = 900;

private:
    void handleInput();
    void render();

    const char *backgroundPath = "assets/background.png";
    const char *fontPath = "assets/Morning Routine.otf";
    const char *musicPath = "assets/background.mp3";
    Game *game = nullptr;
    sf::RenderWindow window;
    sf::Sprite backgroundSprite;
    sf::Text titleText;
    sf::Text startText;
    sf::Text exitText;
    bool active;
};