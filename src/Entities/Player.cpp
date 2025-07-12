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

#include "Entities/Player.hpp"
#include "Core/Constants.hpp"
#include "Core/ResourceManager.hpp"
#include <algorithm>

Player::Player()
    : current_texture(0), speed(Constants::PLAYER_SPEED), damaged(false),
      dying(false), current_shot_gap(Constants::PLAYER_SHOT_GAP),
      health(32000.0f), damage(Constants::PLAYER_DAMAGE),
      recover_health(Constants::PLAYER_RECOVER_HEALTH) {
    avail = true;

    for (const auto &image : images)
        ResourceManager::getTexture(image).setSmooth(true);

    sprite.setTexture(ResourceManager::getTexture(images[0]));
    sprite.setPosition(400.0f, 500.0f);
    sprite.setScale(0.64f, 0.64f);
}

void Player::update(float deltaTime) {
    if (dying) {
        health = 0.0f;
        if (deathTimer.hasElapsed(0.4f)) {
            if (destroyFrameIdx < deathImages.size()) {
                sprite.setTexture(ResourceManager::getTexture(
                    deathImages[destroyFrameIdx++]));
                deathTimer.restart();
            } else {
                avail = false;
                dying = false;
            }
        }
        return;
    }

    if (!avail) {
        health = 0.0f;
        return;
    }

    move(deltaTime);

    // Animation update
    if (animationTimer.hasElapsed(0.16f) && !damaged && !dying) {
        current_texture = (current_texture + 1) % images.size();
        sprite.setTexture(ResourceManager::getTexture(images[current_texture]));
        animationTimer.restart();
    } else if (animationTimer.hasElapsed(0.64f) && damaged) {
        damaged = false;
        animationTimer.restart();
    }

    // Health recovery
    if (recoverTimer.hasElapsed(0.64f) && !dying) {
        health = std::min(max_health, health + recover_health);
        recoverTimer.restart();
    }

    // Check death
    if (health <= 0.0f && !dying) {
        health = 0.0f;
        dying = true;
        destroyFrameIdx = 0;
        sprite.setTexture(ResourceManager::getTexture(deathImages[0]));
        deathTimer.restart();
    }
}

void Player::move(float deltaTime) {
    auto [x, y] = sprite.getPosition();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && x > 0)
        sprite.move(-speed * deltaTime, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
        x < Constants::SCREEN_WIDTH - sprite.getGlobalBounds().width)
        sprite.move(speed * deltaTime, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && y > 0)
        sprite.move(0, -speed * deltaTime);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) &&
        y < Constants::SCREEN_HEIGHT - sprite.getGlobalBounds().height)
        sprite.move(0, speed * deltaTime);
}

void Player::updateCollisions(std::vector<Bullet> &bullet_pool,
                              std::vector<std::unique_ptr<Enemy>> &enemy_pool) {
    if (!avail || dying)
        return;

    const auto bounds = getBounds();

    // Bullet collisions
    for (Bullet &bullet : bullet_pool) {
        if (bullet.isAvailable() && !bullet.from_player &&
            bounds.intersects(bullet.getBounds())) {
            takeDamage(bullet.damage);
            bullet.setAvailable(false);
        }
    }

    // Enemy collisions
    for (auto &enemy : enemy_pool) {
        if (enemy->isAvailable() && enemy->health > 0.0f &&
            bounds.intersects(enemy->getBounds())) {
            takeDamage(enemy->health);
            enemy->collide();
        }
    }
}

void Player::shoot(std::vector<Bullet> &bullet_pool) {
    if (!avail)
        return;

    if (current_shot_gap > 0.0f && !lastShotTimer.hasElapsed(current_shot_gap))
        return;

    bullet_pool.emplace_back(sprite.getPosition() + sf::Vector2f(32.0f, -8.0f),
                             Constants::PLAYER_BULLET_ID, true, 1024.0f,
                             damage);

    lastShotTimer.restart();
}

void Player::takeDamage(float damage) {
    damage = std::max(0.01f, damage);
    health -= damage;
    damaged = true;
    sprite.setTexture(ResourceManager::getTexture("assets/me_hit.png"));
}