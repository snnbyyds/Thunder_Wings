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
#include "Core/Timer.hpp"
#include "Game.hpp"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>

// clang-format off
#define MENU_OPTION_START 0
#define MENU_OPTION_LOAD  1
#define MENU_OPTION_GUIDE 2
#define MENU_OPTION_EXIT  3
#define MENU_MAX_OPTION   MENU_OPTION_EXIT
#define MENU_MIN_OPTION   MENU_OPTION_START
// clang-format on

class Menu {
public:
    Menu();
    ~Menu();
    void show();
    void playLogo();

    static constexpr int screen_w = Constants::SCREEN_WIDTH;
    static constexpr int screen_h = Constants::SCREEN_HEIGHT;

    int currentOption = MENU_OPTION_START;

private:
    void start();
    void load();
    void showGuide();
    void exit();

    void handleInput();
    void render();

    const char *backgroundPath = "assets/background.png";
    const char *fontPath = "assets/Morning Routine.otf";
    const char *musicPath = "assets/background.wav";

    std::unique_ptr<Game> game;
    sf::RenderWindow window;
    sf::Sprite backgroundSprite;

    sf::Text guideText;
    sf::Text loadText;
    sf::Text titleText;
    sf::Text startText;
    sf::Text exitText;

    const float LOGO_DURATION = 3.0f;
    sf::Sprite logoSprite;
    Timer logoClock;
    bool showingLogo = false;

    bool terminated = false;
    bool active;

    bool musicStarted = false;
};