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
#include "Core/ResourceManager.hpp"
#include <algorithm>
#include <cstdlib>

Enemy::Enemy(int level, sf::Vector2f position) : level(level), downFrameIdx(1) {
    avail = true;

    switch (level) {
        case 1:
            health = 1024.0f;
            speed = 128.0f + rand() % 128;
            current_shot_gap = 1.0f + rand() % 2;
            damage = 256.0f * (float)(rand() % 25 + 75) / 100.0f;
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

Enemy1::Enemy1(sf::Vector2f position) : Enemy(1, position) {}