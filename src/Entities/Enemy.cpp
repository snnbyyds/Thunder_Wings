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

#include "Entities/Enemy.hpp"
#include "Core/Constants.hpp"
#include "Core/RandomUtils.hpp"
#include "Core/ResourceManager.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>

Enemy::Enemy(int level, sf::Vector2f position) : level(level), downFrameIdx(1) {
    avail = true;

    switch (level) {
        case 1:
            health = 1024.0f;
            speed = RandomUtils::generateInRange(128.0f, 256.0f);
            current_shot_gap = RandomUtils::generateInRange(0.8f, 1.6f);
            damage = 256.0f;
            break;
        case 2:
            health = 2048.0f;
            speed = RandomUtils::generateInRange(64.0f, 256.0f);
            current_shot_gap = RandomUtils::generateInRange(0.6f, 1.2f);
            damage = 512.0f;
            break;
        default: __builtin_unreachable(); break;
    }

    sprite.setTexture(ResourceManager::getTexture(
        std::string("assets/enemy") + std::to_string(level) + ".png"));
    sprite.setPosition(position);
}

void Enemy::update(float deltaTime) {
    if (!avail)
        return;

    move(deltaTime);

    if (health <= 0.0f) {
        std::string current = std::string("assets/enemy") +
                              std::to_string(level) + "_down" +
                              std::to_string(downFrameIdx) + ".png";

        if (ResourceManager::getTextureifExists(current)) {
            if (animationTimer.hasElapsed(0.16f)) {
                sprite.setTexture(ResourceManager::getTexture(current));
                animationTimer.restart();
                downFrameIdx++;
            }
        } else {
            avail = false;
        }
    }
}

void Enemy::move(float deltaTime) {
    if (!avail)
        return;

    sprite.move(0, speed * deltaTime);
    auto [x, y] = sprite.getPosition();
    avail = (x >= 0 && x < Constants::SCREEN_WIDTH && y >= 0 &&
             y <= Constants::SCREEN_HEIGHT);
}

void Enemy::shoot(std::vector<Bullet> &bullet_pool) {
    if (!avail || health <= 0.0f)
        return;

    if (current_shot_gap > 0.0f && !lastShotTimer.hasElapsed(current_shot_gap))
        return;

    bullet_pool.emplace_back(sprite.getPosition() + sf::Vector2f(32.0f, 8.0f),
                             sf::Vector2f(0.0f, 1.0f), 0, false,
                             std::min(625.0f, speed * 3.0f), damage);

    lastShotTimer.restart();
}

void Enemy::takeDamage(float damage) {
    health -= damage;
    sprite.setTexture(ResourceManager::getTexture(
        std::string("assets/enemy") + std::to_string(level) + "_hit.png"));
}

void Enemy::collide() { takeDamage(health); }

void Enemy::updateBulletCollisions(std::vector<Bullet> &bullet_pool) {
    if (!avail || health <= 0.0f)
        return;

    const auto bounds = getBounds();
    for (Bullet &bullet : bullet_pool) {
        if (bullet.isAvailable() && bullet.from_player &&
            bounds.intersects(bullet.getBounds())) {
            takeDamage(bullet.damage);
            bullet.setAvailable(false);
        }
    }
}

/* Enemy1 Implementation */

Enemy1::Enemy1(sf::Vector2f position) : Enemy(1, position) {}

/* Enemy2 Implementation */

Enemy2::Enemy2(sf::Vector2f position) : Enemy(2, position) {
    verticalAmplitude = RandomUtils::generateInRange(128.0f, 256.0f);
    verticalFrequency = RandomUtils::generateInRange(0.5f, 1.5f);
    verticalCenter = position.x;
    timer.restart();
}

void Enemy2::move(float deltaTime) {
    if (!avail)
        return;

    float verticalOffset =
        std::sin(timer.getElapsedTime() * verticalFrequency) *
        verticalAmplitude;
    sprite.move(0, speed * deltaTime);
    auto [x, y] = sprite.getPosition();
    sprite.setPosition(verticalCenter + verticalOffset, y);
    avail = (y >= 0 && y <= Constants::SCREEN_HEIGHT);
}

void Enemy2::collide() { takeDamage(health * 0.8f); }
