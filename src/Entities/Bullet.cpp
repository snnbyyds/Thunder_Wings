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
    : from_player(from_player), damage(damage), speed(speed) {
    avail = true;
    size_t bullets_path_size = sizeof(bullets_path) / sizeof(*bullets_path);
    id = std::min(id, bullets_path_size - 1);
    sprite.setTexture(ResourceManager::getTexture(bullets_path[id]));
    sprite.setPosition(position);
    direction = sf::Vector2f(0.0f, -1.0f);
    if (id == Constants::ENEMY_MISSILE_ID)
        sprite.setOrigin(sprite.getLocalBounds().width / 2,
                         sprite.getLocalBounds().height);
}

Bullet::Bullet(sf::Vector2f position, sf::Vector2f direction, size_t id,
               bool from_player, float speed, float damage)
    : Bullet(position, id, from_player, speed, damage) {
    this->direction = Math::normalize(direction);
    this->tracking = 0.0f;
}

Bullet::Bullet(sf::Vector2f position, sf::Vector2f direction, size_t id,
               bool from_player, float speed, float damage, float tracking)
    : Bullet(position, direction, id, from_player, speed, damage) {
    if (from_player)
        tracking = 0.0f; // Disable tracking for player bullets
    this->tracking = tracking;
    if (id == Constants::ENEMY_MISSILE_ID)
        updateRotation();
}

void Bullet::updateRotation() {
    float angle = Math::vectorToAngle(direction) * Math::RAD_TO_DEG + 90.0f;
    sprite.setRotation(angle);
}

void Bullet::update(float deltaTime, sf::Vector2f hitTarget) {
    if (!avail)
        return;

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
