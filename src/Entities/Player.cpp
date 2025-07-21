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
    : damaged(false), dying(false),
      current_shot_gap(Constants::PLAYER_SHOT_GAP),
      health(Constants::PLAYER_MAX_HEALTH * 0.64f),
      damage(Constants::PLAYER_DAMAGE),
      recover_health(Constants::PLAYER_RECOVER_HEALTH),
      speed(Constants::PLAYER_SPEED), current_texture(0) {
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
                ResourceManager::playSound("assets/me_down.wav");
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

void Player::updateCollisions(
    std::vector<std::unique_ptr<Bullet>> &bullet_pool) {
    if (!avail || dying)
        return;

    const auto bounds = getBounds();

    // Bullet collisions
    for (auto &bullet : bullet_pool) {
        if (bullet->isAvailable() && !bullet->from_player &&
            bounds.intersects(bullet->getBounds())) {
            takeDamage(std::max(bullet->damage, bullet->damageRate * health));
            bullet->explode();
            bullet->setAvailable(false);
        }
    }
}

void Player::shoot(std::vector<std::unique_ptr<Bullet>> &bullet_pool) {
    if (!avail)
        return;

    if (current_shot_gap > 0.0f) {
        float shot_gap = current_shot_gap;
        float multiplier = 1.0f;
        for (auto &gift : gifts)
            multiplier *= (1.0f + gift->attackSpeedIncrease);
        shot_gap = current_shot_gap / multiplier;
        if (!lastShotTimer.hasElapsed(shot_gap))
            return;
    }

    const sf::Vector2f playerCenter =
        sprite.getPosition() +
        sf::Vector2f(sprite.getGlobalBounds().width / 2.0f, 0.0f);

    const float horizontalOffset = 20.0f;
    const float verticalOffset = -8.0f;

    // left
    bullet_pool.push_back(std::make_unique<Cannon>(
        playerCenter + sf::Vector2f(-horizontalOffset, verticalOffset),
        sf::Vector2f(0.0f, -1.0f), Constants::PLAYER_BULLET_ID, true, 1024.0f,
        damage));

    // right
    bullet_pool.push_back(std::make_unique<Cannon>(
        playerCenter + sf::Vector2f(horizontalOffset, verticalOffset),
        sf::Vector2f(0.0f, -1.0f), Constants::PLAYER_BULLET_ID, true, 1024.0f,
        damage));

    // ResourceManager::playSound("assets/bullet.wav");

    lastShotTimer.restart();
}

void Player::takeDamage(float rawDamage) {
    float dmgMul = 1.0f;
    for (auto &gift : gifts)
        dmgMul *= (1.0f - gift->damageReduction);

    float finalDamage = rawDamage * dmgMul;
    finalDamage = std::max(0.0f, finalDamage);

    health -= finalDamage;
    damaged = true;
    sprite.setTexture(ResourceManager::getTexture("assets/me_hit.png"));
}

boost::json::object Player::serialize() const {
    boost::json::object o = Entity::serialize();
    o["damaged"] = damaged;
    o["current_shot_gap"] = current_shot_gap;
    o["health"] = health;
    o["damage"] = damage;
    o["recover_health"] = recover_health;
    return o;
}

void Player::deserialize(const boost::json::object &o) {
    Entity::deserialize(o);
    damaged = o.at("damaged").as_bool();
    current_shot_gap = (float)o.at("current_shot_gap").as_double();
    health = (float)o.at("health").as_double();
    damage = (float)o.at("damage").as_double();
    recover_health = (float)o.at("recover_health").as_double();
}
