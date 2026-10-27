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

#include "thunder/assets/AssetManager.hpp"
#include "thunder/game/GameWorld.hpp"

#include <SFML/Graphics.hpp>

#include <string_view>
#include <vector>

namespace thunder {

class GameRenderer {
public:
    explicit GameRenderer(const AssetManager &assets);

    void draw(sf::RenderTarget &target, const GameWorld &world);
    void drawPauseOverlay(sf::RenderTarget &target, int selectedOption, std::string_view message);
    void drawGameOver(sf::RenderTarget &target);

private:
    void drawPlayer(sf::RenderTarget &target, const GameWorld &world);
    void drawBullets(sf::RenderTarget &target, const GameWorld &world);
    void drawExplosions(sf::RenderTarget &target, const GameWorld &world);
    void drawEnemies(sf::RenderTarget &target, const GameWorld &world);
    void drawAllies(sf::RenderTarget &target, const GameWorld &world);
    void drawHud(sf::RenderTarget &target, const GameWorld &world);
    void drawGifts(sf::RenderTarget &target, const GameWorld &world);

    const AssetManager &assets_;

    sf::Text timeText_;
    sf::Text healthText_;
    sf::Text killedText_;
    sf::Text bossText_;
    sf::Text instructionText_;
    sf::Text pauseText_;
    sf::Text resumeText_;
    sf::Text saveText_;
    sf::Text exitText_;
    sf::Text gameOverText_;
    sf::Text messageText_;
    std::vector<sf::Text> giftTexts_;
};

} // namespace thunder
