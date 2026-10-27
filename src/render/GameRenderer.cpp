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

#include "thunder/render/GameRenderer.hpp"

#include "thunder/core/Math.hpp"
#include "thunder/game/Config.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <numbers>

namespace thunder {
namespace {

void centerAt(sf::Text &text, float x, float y) {
    const auto bounds = text.getLocalBounds();
    text.setOrigin(
        {bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f});
    text.setPosition({x, y});
}

sf::Color giftTimerColor(float remaining) {
    if (remaining < 2.0f)
        return sf::Color::Red;
    if (remaining < 5.0f)
        return sf::Color::Yellow;
    return sf::Color::Green;
}

} // namespace

GameRenderer::GameRenderer(const AssetManager &assets)
    : assets_(assets), timeText_(assets.font(FontId::Game), "", 30),
      healthText_(assets.font(FontId::Game), "", 30),
      killedText_(assets.font(FontId::Game), "", 32), bossText_(assets.font(FontId::Game), "", 80),
      instructionText_(assets.font(FontId::Game), "Use Arrow Keys to Move", 40),
      pauseText_(assets.font(FontId::Game), "PAUSED", 80),
      resumeText_(assets.font(FontId::Game), "Resume", 60),
      saveText_(assets.font(FontId::Game), "Save Progress", 60),
      exitText_(assets.font(FontId::Game), "Exit", 60),
      gameOverText_(assets.font(FontId::Game), "GAME OVER", 80),
      messageText_(assets.font(FontId::Game), "", 26) {
    timeText_.setFillColor(sf::Color::Red);
    timeText_.setPosition({static_cast<float>(config::kScreenWidth) - 200.0f, 10.0f});

    healthText_.setFillColor(sf::Color::Yellow);
    healthText_.setPosition({static_cast<float>(config::kScreenWidth) - 280.0f, 40.0f});

    killedText_.setFillColor(sf::Color::Green);
    killedText_.setPosition({static_cast<float>(config::kScreenWidth) - 200.0f, 80.0f});

    bossText_.setFillColor(sf::Color::Magenta);
    bossText_.setPosition({static_cast<float>(config::kScreenWidth) / 2.0f - 200.0f, 20.0f});

    instructionText_.setFillColor(sf::Color::Green);
    instructionText_.setStyle(sf::Text::Bold);
    centerAt(instructionText_, static_cast<float>(config::kScreenWidth) / 2.0f,
             static_cast<float>(config::kScreenHeight) - 60.0f);

    pauseText_.setFillColor(sf::Color::Cyan);
    pauseText_.setStyle(sf::Text::Bold);
    centerAt(pauseText_, static_cast<float>(config::kScreenWidth) / 2.0f,
             static_cast<float>(config::kScreenHeight) / 2.0f - 350.0f);

    centerAt(resumeText_, static_cast<float>(config::kScreenWidth) / 2.0f,
             pauseText_.getPosition().y + 260.0f);
    centerAt(saveText_, static_cast<float>(config::kScreenWidth) / 2.0f,
             resumeText_.getPosition().y + 80.0f);
    centerAt(exitText_, static_cast<float>(config::kScreenWidth) / 2.0f,
             saveText_.getPosition().y + 80.0f);

    gameOverText_.setFillColor(sf::Color::Red);
    centerAt(gameOverText_, static_cast<float>(config::kScreenWidth) / 2.0f,
             static_cast<float>(config::kScreenHeight) / 2.0f);

    messageText_.setFillColor(sf::Color::Yellow);
    centerAt(messageText_, static_cast<float>(config::kScreenWidth) / 2.0f, 640.0f);

    giftTexts_.reserve(config::kMaxActiveGifts);
    for (std::size_t i = 0; i < config::kMaxActiveGifts; ++i)
        giftTexts_.emplace_back(assets_.font(FontId::Game), "", 26);
}

void GameRenderer::draw(sf::RenderTarget &target, const GameWorld &world) {
    sf::Sprite background(assets_.texture(TextureId::Background));
    background.setColor(world.allMyPeopleActive() ? sf::Color(244, 154, 240, 232)
                                                  : sf::Color::White);
    target.draw(background);

    drawPlayer(target, world);
    drawBullets(target, world);
    drawExplosions(target, world);
    drawEnemies(target, world);
    drawAllies(target, world);
    drawHud(target, world);
    drawGifts(target, world);

    if (world.elapsedTime() <= config::kInstructionsDuration)
        target.draw(instructionText_);
}

void GameRenderer::drawPlayer(sf::RenderTarget &target, const GameWorld &world) {
    const Player &player = world.player();
    TextureId texture = TextureId::Player1;
    if (player.dying || player.dead)
        texture = playerDeathTexture(player.deathFrame);
    else
        texture = player.textureIndex == 0 ? TextureId::Player1 : TextureId::Player2;

    const sf::Texture &playerTexture = assets_.texture(texture);
    sf::Sprite sprite(playerTexture);
    const auto size = playerTexture.getSize();
    sprite.setOrigin({static_cast<float>(size.x) / 2.0f, static_cast<float>(size.y) / 2.0f});
    sprite.setScale({config::kPlayerSpriteScale, config::kPlayerSpriteScale});
    sprite.setPosition({player.position.x, player.position.y});
    sprite.setColor(sf::Color::Cyan);
    target.draw(sprite);

    if (!player.dying && !player.dead &&
        std::any_of(player.gifts.begin(), player.gifts.end(), [](const Gift &gift) {
            return gift.kind == GiftKind::CenturyShield && gift.remaining > 0.0f;
        })) {
        const sf::Texture &shieldTexture = assets_.texture(TextureId::PlayerShield);
        sf::Sprite shield(shieldTexture);
        const auto shieldSize = shieldTexture.getSize();
        shield.setOrigin(
            {static_cast<float>(shieldSize.x) / 2.0f, static_cast<float>(shieldSize.y) / 2.0f});
        shield.setScale({config::kPlayerSpriteScale, config::kPlayerSpriteScale});
        shield.setPosition({player.position.x, player.position.y});
        shield.setRotation(sf::degrees(std::fmod(world.elapsedTime() * 90.0f, 360.0f)));
        target.draw(shield);
    }
}

void GameRenderer::drawBullets(sf::RenderTarget &target, const GameWorld &world) {
    for (const Bullet &bullet : world.bullets()) {
        const sf::Texture &texture = assets_.texture(bulletTexture(bullet.visual));
        sf::Sprite sprite(texture);
        const auto size = texture.getSize();
        if (bullet.visual == BulletVisual::Missile || bullet.visual == BulletVisual::Rocket) {
            sprite.setOrigin({static_cast<float>(size.x) / 2.0f, static_cast<float>(size.y)});
        } else {
            sprite.setOrigin(
                {static_cast<float>(size.x) / 2.0f, static_cast<float>(size.y) / 2.0f});
        }
        sprite.setPosition({bullet.position.x, bullet.position.y});
        const float degrees = std::atan2(bullet.direction.y, bullet.direction.x) *
                              (180.0f / std::numbers::pi_v<float>)+90.0f;
        sprite.setRotation(sf::degrees(degrees));
        target.draw(sprite);
    }
}

void GameRenderer::drawExplosions(sf::RenderTarget &target, const GameWorld &world) {
    for (const Explosion &explosion : world.explosions()) {
        if (explosion.timer > config::kExplosionFlashDuration) {
            sf::RectangleShape flash({static_cast<float>(config::kScreenWidth),
                                      static_cast<float>(config::kScreenHeight)});
            flash.setFillColor(explosion.kind == ExplosionKind::Missile
                                   ? sf::Color(255, 255, 255, 220)
                                   : sf::Color(255, 69, 1, 128));
            target.draw(flash);
        }

        const TextureId texture = explosion.kind == ExplosionKind::Missile
                                      ? TextureId::ExplosionMissile
                                      : TextureId::ExplosionRocket;
        sf::Sprite sprite(assets_.texture(texture));
        target.draw(sprite);
    }
}

void GameRenderer::drawEnemies(sf::RenderTarget &target, const GameWorld &world) {
    for (const Enemy &enemy : world.enemies()) {
        TextureId texture = enemyTexture(enemy.type);
        if (enemy.dying)
            texture = enemyDeathTexture(enemy.type, enemy.deathFrame);
        else if (enemy.hitFlash > 0.0f)
            texture = enemyHitTexture(enemy.type);

        sf::Sprite sprite(assets_.texture(texture));
        sprite.setPosition({enemy.position.x, enemy.position.y});
        sprite.setColor(sf::Color::Yellow);
        target.draw(sprite);
    }
}

void GameRenderer::drawAllies(sf::RenderTarget &target, const GameWorld &world) {
    const sf::Texture &texture = assets_.texture(TextureId::Enemy1);
    const auto size = texture.getSize();
    for (const Ally &ally : world.allies()) {
        sf::Sprite sprite(texture);
        sprite.setOrigin({static_cast<float>(size.x) / 2.0f, static_cast<float>(size.y) / 2.0f});
        sprite.setPosition({ally.position.x, ally.position.y});
        sprite.setRotation(sf::degrees(180.0f));
        sprite.setColor(sf::Color::Cyan);
        target.draw(sprite);
    }
}

void GameRenderer::drawHud(sf::RenderTarget &target, const GameWorld &world) {
    timeText_.setString(std::format("Time: {:.3f}s", world.elapsedTime()));
    healthText_.setString(std::format("Health: {:.2f}", world.player().health));
    killedText_.setString(std::format("killed: {}", world.kills()));

    target.draw(timeText_);
    target.draw(healthText_);
    target.draw(killedText_);

    if (const Enemy *boss = world.activeBoss()) {
        bossText_.setString(std::format("Boss: {}\n{:.5f}%", static_cast<int>(boss->health),
                                        boss->health / boss->maxHealth * 100.0f));
        target.draw(bossText_);
    }
}

void GameRenderer::drawGifts(sf::RenderTarget &target, const GameWorld &world) {
    const auto gifts = world.gifts();
    if (gifts.empty())
        return;

    constexpr float padding = 20.0f;
    constexpr float iconSize = 80.0f;
    constexpr float spacing = 15.0f;
    constexpr float textGap = 8.0f;
    constexpr float baseY = static_cast<float>(config::kScreenHeight) - iconSize - padding - 40.0f;

    const std::size_t count = std::min(gifts.size(), giftTexts_.size());
    for (std::size_t i = 0; i < count; ++i) {
        const Gift &gift = gifts[i];
        const float x = padding + static_cast<float>(i) * (iconSize + spacing);
        const float y = baseY;

        sf::RectangleShape background({iconSize + 8.0f, iconSize + 40.0f + 8.0f});
        background.setPosition({x - 4.0f, y - 4.0f});
        background.setFillColor(sf::Color(0, 0, 0, 120));
        background.setOutlineThickness(2.0f);
        background.setOutlineColor(sf::Color(255, 255, 255, 180));
        target.draw(background);

        const sf::Texture &texture = assets_.texture(giftTexture(gift.kind));
        sf::Sprite icon(texture);
        const auto size = texture.getSize();
        const float scale = iconSize / static_cast<float>(size.y);
        icon.setScale({scale, scale});
        icon.setPosition({x, y});
        target.draw(icon);

        sf::Text &timer = giftTexts_[i];
        timer.setString(std::format("{:.1f}s", gift.remaining));
        timer.setFillColor(giftTimerColor(gift.remaining));
        const auto bounds = timer.getLocalBounds();
        timer.setPosition({x + (iconSize - bounds.size.x) / 2.0f, y + iconSize + textGap});
        target.draw(timer);

        if (gift.remaining < 3.0f) {
            const float blinkSpeed = gift.remaining < 1.0f ? 4.0f : 2.0f;
            if (std::fmod(world.elapsedTime() * blinkSpeed, 2.0f) < 1.0f) {
                sf::RectangleShape alert({iconSize, iconSize});
                alert.setPosition({x, y});
                alert.setFillColor(sf::Color(255, 100, 100, 80));
                target.draw(alert);
            }
        }

        if (gift.maxTime > 0.0f) {
            const float progress = clamp(gift.remaining / gift.maxTime, 0.0f, 1.0f);
            sf::RectangleShape progressBackground({iconSize, 4.0f});
            progressBackground.setPosition({x, y + iconSize + 2.0f});
            progressBackground.setFillColor(sf::Color(50, 50, 50, 200));
            target.draw(progressBackground);

            sf::RectangleShape progressBar({iconSize * progress, 4.0f});
            progressBar.setPosition({x, y + iconSize + 2.0f});
            if (progress < 0.2f)
                progressBar.setFillColor(sf::Color::Red);
            else if (progress < 0.5f)
                progressBar.setFillColor(sf::Color::Yellow);
            else
                progressBar.setFillColor(sf::Color::Green);
            target.draw(progressBar);
        }
    }
}

void GameRenderer::drawPauseOverlay(sf::RenderTarget &target, int selectedOption,
                                    std::string_view message) {
    sf::RectangleShape overlay(
        {static_cast<float>(config::kScreenWidth), static_cast<float>(config::kScreenHeight)});
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    target.draw(overlay);
    target.draw(pauseText_);

    resumeText_.setFillColor(selectedOption == 0 ? sf::Color::Blue : sf::Color::White);
    saveText_.setFillColor(selectedOption == 1 ? sf::Color::Blue : sf::Color::White);
    exitText_.setFillColor(selectedOption == 2 ? sf::Color::Red : sf::Color::White);
    target.draw(resumeText_);
    target.draw(saveText_);
    target.draw(exitText_);

    if (!message.empty()) {
        messageText_.setString(std::string(message));
        centerAt(messageText_, static_cast<float>(config::kScreenWidth) / 2.0f, 640.0f);
        target.draw(messageText_);
    }
}

void GameRenderer::drawGameOver(sf::RenderTarget &target) {
    target.draw(gameOverText_);
}

} // namespace thunder
