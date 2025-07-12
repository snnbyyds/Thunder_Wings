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
#include <cmath>
#include <cstdlib>

Enemy::Enemy(int level, sf::Vector2f position) : level(level), downFrameIdx(1) {
    avail = true;

    switch (level) {
        case 1:
            maxHealth = health = 2048.0f;
            speed = RandomUtils::generateInRange(128.0f, 256.0f);
            bulletspeed = speed * 3.2f;
            current_shot_gap = RandomUtils::generateInRange(0.8f, 1.6f);
            damage = 128.0f;
            killBonus = 512.8f;
            break;
        case 2:
            maxHealth = health = 16384.0f;
            speed = RandomUtils::generateInRange(64.0f, 256.0f);
            bulletspeed = speed * 4.0f;
            current_shot_gap = RandomUtils::generateInRange(0.6f, 1.2f);
            damage = 1024.0f;
            killBonus = 3072.9f;
            break;
        case 3:
            maxHealth = health =
                RandomUtils::generateInRange(131072.0f, 524288.0f);
            speed = RandomUtils::generateInRange(8.0f, 32.0f);
            bulletspeed = RandomUtils::generateInRange(1024.0f, 2048.f);
            current_shot_gap = RandomUtils::generateInRange(0.08f, 0.4f);
            damage = 1024.0f;
            killBonus = maxHealth;
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

    const sf::FloatRect bounds = sprite.getGlobalBounds();
    sf::Vector2f spawnPosition(bounds.left + bounds.width / 2.0f,
                               bounds.top + bounds.height + 8.0f);
    sf::Vector2f direction = {0.0f, 1.0f};
    bullet_pool.emplace_back(spawnPosition, direction,
                             Constants::ENEMY_BULLET_ID, false, bulletspeed,
                             damage);

    lastShotTimer.restart();
}

void Enemy::takeDamage(float damage) {
    health -= damage;
    sprite.setTexture(ResourceManager::getTexture(
        std::string("assets/enemy") + std::to_string(level) + "_hit.png"));
}

void Enemy::collide() {
    if (!lastCollideTimer.hasElapsed(0.02)) {
        lastCollideTimer.restart();
        takeDamage(640.0f);
    }
}

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

void Enemy2::collide() {
    if (!lastCollideTimer.hasElapsed(0.02)) {
        lastCollideTimer.restart();
        takeDamage(320.0f);
    }
}

/* Enemy3 Implementation */

Enemy3::Enemy3(sf::Vector2f position) : Enemy(3, position) {
    verticalAmplitude = RandomUtils::generateInRange(128.0f, 256.0f);
    verticalFrequency = RandomUtils::generateInRange(0.6f, 1.2f);
    verticalCenter = position.x;
    timer.restart();
}

void Enemy3::move(float deltaTime) {
    if (!avail)
        return;

    float verticalOffset =
        std::sin(timer.getElapsedTime() * verticalFrequency) *
        verticalAmplitude;
    if (sprite.getPosition().y > (float)Constants::SCREEN_HEIGHT / 4)
        sprite.move(0, speed * deltaTime);
    auto [x, y] = sprite.getPosition();
    sprite.setPosition(verticalCenter + verticalOffset, y);
    avail = (x >= 0 && x < Constants::SCREEN_WIDTH && y >= 0 &&
             y <= Constants::SCREEN_HEIGHT);
}

void Enemy3::collide() { 
    if (!lastCollideTimer.hasElapsed(0.02)) {
        lastCollideTimer.restart();
        takeDamage(64.0f);
    }
}

void Enemy3::shoot(std::vector<Bullet> &bullet_pool) {
    if (!avail || health <= 0.0f)
        return;

    if (current_shot_gap > 0.0f && !lastShotTimer.hasElapsed(current_shot_gap))
        return;

    static int shootCounter = 0;

    const sf::FloatRect bounds = sprite.getGlobalBounds();
    const float centerX = bounds.left + bounds.width / 2.0f;
    const float bottomY = bounds.top + bounds.height + 8.0f;
    const float spacing = 20.0f; // Spacing for bullets

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            const float offsetX = (col - 1) * spacing;
            const float offsetY = (row - 1) * spacing;

            sf::Vector2f spawnPosition(centerX + offsetX, bottomY + offsetY);

            float bulletDamage = damage;
            if (col == 0 || col == 2)
                bulletDamage = damage * 0.333f;

            bullet_pool.emplace_back(spawnPosition, sf::Vector2f(0.0f, 1.0f),
                                     Constants::ENEMY_BULLET_ID, false,
                                     bulletspeed, bulletDamage);
        }
    }

    shootCounter = (shootCounter + 1) % 12;
    if (!shootCounter) {
        for (int i = 0; i < 3; i++) {
            bullet_pool.emplace_back(
                sf::Vector2f(centerX - 50.0f - i * 20.0f, bottomY - i * 36.0f),
                sf::Vector2f(0.0f, 1.0f), Constants::ENEMY_MISSILE_ID, false,
                bulletspeed * (0.12f + i * 0.01f), damage * 4.8f,
                0.4f + i * 0.5f);

            bullet_pool.emplace_back(
                sf::Vector2f(centerX + 50.0f + i * 20.0f, bottomY - i * 36.0f),
                sf::Vector2f(0.0f, 1.0f), Constants::ENEMY_MISSILE_ID, false,
                bulletspeed * (0.12f + i * 0.01f), damage * 4.8f,
                0.4f + i * 0.5f);
        }
    }

    lastShotTimer.restart();
}
