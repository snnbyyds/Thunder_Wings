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

#include "thunder/render/MenuRenderer.hpp"

#include "thunder/game/Config.hpp"

#include <algorithm>
#include <cstdint>

namespace thunder {
namespace {

void centerAt(sf::Text &text, float x, float y) {
    const auto bounds = text.getLocalBounds();
    text.setOrigin(
        {bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
    text.setPosition({x, y});
}

} // namespace

MenuRenderer::MenuRenderer(const AssetManager &assets)
    : assets_(assets), titleText_(assets.font(FontId::Game), "THUNDER WINGS", 80),
      startText_(assets.font(FontId::Game), "START", 50),
      loadText_(assets.font(FontId::Game), "LOAD PROGRESS", 50),
      guideText_(assets.font(FontId::Game), "GUIDE", 50),
      aboutText_(assets.font(FontId::Game), "ABOUT", 50),
      exitText_(assets.font(FontId::Game), "EXIT", 50),
      messageText_(assets.font(FontId::Game), "", 24) {
    titleText_.setFillColor(sf::Color(255, 215, 0));
    titleText_.setStyle(sf::Text::Bold);

    const float centerX = static_cast<float>(config::kScreenWidth) / 2.0f;
    centerAt(titleText_, centerX, 100.0f);
    centerAt(startText_, centerX, 300.0f);
    centerAt(loadText_, centerX, 400.0f);
    centerAt(guideText_, centerX, 500.0f);
    centerAt(aboutText_, centerX, 600.0f);
    centerAt(exitText_, centerX, 700.0f);

    messageText_.setFillColor(sf::Color::Yellow);
    messageText_.setStyle(sf::Text::Italic);
}

void MenuRenderer::drawBackground(sf::RenderTarget &target) {
    sf::Sprite background(assets_.texture(TextureId::Background));
    target.draw(background);
}

void MenuRenderer::drawMenu(sf::RenderTarget &target, int selectedOption, bool saveAvailable,
                            std::string_view message) {
    drawBackground(target);
    target.draw(titleText_);

    startText_.setFillColor(selectedOption == 0 ? sf::Color::Green : sf::Color::White);
    loadText_.setFillColor(!saveAvailable
                               ? sf::Color(128, 128, 128)
                               : (selectedOption == 1 ? sf::Color::Yellow : sf::Color::White));
    guideText_.setFillColor(selectedOption == 2 ? sf::Color::Yellow : sf::Color::White);
    aboutText_.setFillColor(selectedOption == 3 ? sf::Color::Yellow : sf::Color::White);
    exitText_.setFillColor(selectedOption == 4 ? sf::Color::Red : sf::Color::White);

    target.draw(startText_);
    target.draw(loadText_);
    target.draw(guideText_);
    target.draw(aboutText_);
    target.draw(exitText_);

    if (!message.empty()) {
        messageText_.setString(std::string(message));
        centerAt(messageText_, static_cast<float>(config::kScreenWidth) / 2.0f, 790.0f);
        target.draw(messageText_);
    }
}

void MenuRenderer::drawPage(sf::RenderTarget &target, std::span<const std::string> lines) {
    drawBackground(target);

    float y = 8.0f;
    for (const auto &line : lines) {
        const bool header = line.rfind("--", 0) == 0;
        const unsigned int characterSize = header ? 28U : 24U;
        sf::Text text(assets_.font(FontId::Page), line, characterSize);
        text.setFillColor(header ? sf::Color::Red : sf::Color::Black);
        text.setPosition({50.0f, y});
        target.draw(text);
        y += static_cast<float>(characterSize) + (header ? 14.0f : 10.0f);
    }
}

} // namespace thunder
