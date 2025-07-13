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

#include "Entities/Bullet.hpp"
#include "Core/Constants.hpp"
#include "Core/Math.hpp"
#include "Core/ResourceManager.hpp"
#include <algorithm>

Bullet::Bullet(sf::Vector2f position, size_t id, bool from_player, float speed,
               float damage)
    : from_player(from_player), damage(damage), speed(speed), id(id), exploding(false) {
    avail = true;
    size_t bullets_path_size = sizeof(bullets_path) / sizeof(*bullets_path);
    id = std::min(id, bullets_path_size - 1);
    sprite.setTexture(ResourceManager::getTexture(bullets_path[id]));
    sprite.setPosition(position);
    direction = sf::Vector2f(0.0f, -1.0f);
}

Bullet::Bullet(sf::Vector2f position, sf::Vector2f direction, size_t id,
               bool from_player, float speed, float damage)
    : Bullet(position, id, from_player, speed, damage) {
    this->direction = Math::normalize(direction);
}

void Bullet::updateRotation() {
    float angle = Math::vectorToAngle(direction) * Math::RAD_TO_DEG + 90.0f;
    sprite.setRotation(angle);
}

void Bullet::update(float deltaTime, sf::Vector2f hitTarget) {
    if (!avail)
        return;

    sprite.move(direction * speed * deltaTime);

    auto [x, y] = sprite.getPosition();
    avail = (x >= 0 && x <= Constants::SCREEN_WIDTH && y >= 0 &&
             y <= Constants::SCREEN_HEIGHT);
}

void Bullet::render(sf::RenderWindow &window) {
    if (avail)
        window.draw(sprite);
}

void Bullet::explode() {}

// Cannon

Cannon::Cannon(sf::Vector2f position, size_t id, bool from_player, float speed,
               float damage)
    : Bullet(position, id, from_player, speed, damage) {
}

Cannon::Cannon(sf::Vector2f position, sf::Vector2f direction, size_t id,
               bool from_player, float speed, float damage)
    : Bullet(position, direction, id, from_player, speed, damage) {
}

// Missile

Missile::Missile(sf::Vector2f position, sf::Vector2f direction, size_t id,
                 bool from_player, float speed, float damage, float tracking)
    : Bullet(position, direction, id, from_player, speed, damage),
      tracking(tracking) {
    
    if (from_player)
        this->tracking = 0.0f;
    
    if (id == Constants::ENEMY_MISSILE_ID)
        sprite.setOrigin(sprite.getLocalBounds().width / 2,
                         sprite.getLocalBounds().height);
    
    whiteFlash.setSize(
        sf::Vector2f(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT));
    whiteFlash.setFillColor(sf::Color(255, 255, 255, 220));
    
    if (id == Constants::ENEMY_MISSILE_ID)
        updateRotation();
}

void Missile::update(float deltaTime, sf::Vector2f hitTarget) {
    if (!avail)
        return;

    if (from_player)
        tracking = 0.0f;

    if (tracking > 0.0f) {
        tracking += deltaTime * 0.00002;
        sf::Vector2f targetDir = hitTarget - sprite.getPosition();
        targetDir = Math::normalize(targetDir);

        float currentAngle = Math::vectorToAngle(direction);
        float targetAngle = Math::vectorToAngle(targetDir);
        float angleDiff = Math::normalizeAngle(targetAngle - currentAngle);

        float maxTurn = tracking * Math::PI * deltaTime;
        if (std::abs(angleDiff) > maxTurn)
            angleDiff = (angleDiff > 0) ? maxTurn : -maxTurn;

        float newAngle = currentAngle + angleDiff;
        direction = Math::angleToVector(newAngle);

        updateRotation();
        if (timer.hasElapsed(12.0f))
            tracking = 0.0f;
    }

    sprite.move(direction * speed * deltaTime);

    auto [x, y] = sprite.getPosition();
    avail = (x >= 0 && x <= Constants::SCREEN_WIDTH && y >= 0 &&
             y <= Constants::SCREEN_HEIGHT);
}

void Missile::render(sf::RenderWindow &window) {
    if (exploding) {
        avail = false;
        if (explodeTimer.hasElapsed(0.16f)) {
            exploding = false;
        } else {
            window.draw(whiteFlash);
        }
    }
    if (avail)
        window.draw(sprite);
}

void Missile::explode() {
    if (id == Constants::ENEMY_MISSILE_ID) {
        ResourceManager::playSound("assets/explode.wav");
        exploding = true;
        explodeTimer.restart();
    }
}