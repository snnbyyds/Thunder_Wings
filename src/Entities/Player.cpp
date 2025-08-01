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
#include "Core/Math.hpp"
#include "Core/RandomUtils.hpp"
#include "Core/ResourceManager.hpp"
#include <algorithm>

Player::Player()
    : dying(false), current_shot_gap(Constants::PLAYER_SHOT_GAP),
      health(Constants::PLAYER_MAX_HEALTH * 0.64f),
      damage(Constants::PLAYER_DAMAGE),
      recover_health(Constants::PLAYER_RECOVER_HEALTH), charming(false),
      speed(Constants::PLAYER_SPEED), current_texture(0) {
    avail = true;

    for (const auto &image : images)
        ResourceManager::getTexture(image).setSmooth(true);

    sprite.setTexture(ResourceManager::getTexture(images[0]));
    sf::Vector2u playerSize = sprite.getTexture()->getSize();
    sprite.setOrigin(playerSize.x / 2.0f, playerSize.y / 2.0f);
    sprite.setPosition(400.0f, 500.0f);
    sprite.setScale(0.64f, 0.64f);
    sprite.setColor(sf::Color::Cyan);

    // Configure the shield sprite
    shieldSprite.setTexture(
        ResourceManager::getTexture("assets/PlayerShield.png"));
    sf::Vector2u shieldSize = shieldSprite.getTexture()->getSize();
    shieldSprite.setOrigin(shieldSize.x / 2.0f, shieldSize.y / 2.0f);
    shieldSprite.setScale(0.64f, 0.64f);
    shieldSprite.setPosition(sprite.getPosition());
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
    if (animationTimer.hasElapsed(0.16f) && !dying) {
        current_texture = (current_texture + 1) % images.size();
        sprite.setTexture(ResourceManager::getTexture(images[current_texture]));
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

    // Check gifts
    bool isCharming = false, hasShield = false;
    speedIncrease = 0.0f;
    for (const auto &gift : gifts) {
        speedIncrease += gift->speedIncrease;
        if (gift->charming)
            isCharming = true;
        else if (gift->damageReduction > 0.0f)
            hasShield = true;
    }
    charming = isCharming;
    this->hasShield = hasShield;

    if (hasShield)
        shieldSprite.rotate(90.0f * deltaTime);
}

void Player::render(sf::RenderWindow &window) {
    Entity::render(window);
    if (hasShield) {
        shieldSprite.setPosition(sprite.getPosition());
        window.draw(shieldSprite);
    }
}

void Player::move(float deltaTime) {
    auto [x, y] = sprite.getPosition();

    float realSpeed = speed * (1.0f + speedIncrease);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && x > 0)
        sprite.move(-realSpeed * deltaTime, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) &&
        x < Constants::SCREEN_WIDTH - sprite.getGlobalBounds().width)
        sprite.move(realSpeed * deltaTime, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && y > 0)
        sprite.move(0, -realSpeed * deltaTime);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) &&
        y < Constants::SCREEN_HEIGHT - sprite.getGlobalBounds().height)
        sprite.move(0, realSpeed * deltaTime);
}

void Player::updateCollisions(
    std::vector<std::unique_ptr<Bullet>> &bullet_pool) {
    if (!avail || dying)
        return;

    const auto bounds =
        hasShield ? shieldSprite.getGlobalBounds() : getBounds();

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

    bool shotSpeedIncreased = false;
    if (current_shot_gap > 0.0f) {
        float shot_gap = current_shot_gap;
        float multiplier = 1.0f;
        for (auto &gift : gifts)
            multiplier *= (1.0f + gift->attackSpeedIncrease);
        if (multiplier > 1.0f) {
            shot_gap = current_shot_gap / multiplier;
            shotSpeedIncreased = true;
        }
        if (!lastShotTimer.hasElapsed(shot_gap))
            return;
    }

    const sf::Vector2f playerCenter = sprite.getPosition();
    const float horizontalOffset = 20.0f;
    const float verticalOffset = -16.0f;

    // left
    bullet_pool.push_back(std::make_unique<Cannon>(
        playerCenter + sf::Vector2f(-horizontalOffset, verticalOffset),
        sf::Vector2f(0.0f, -1.0f), Constants::PLAYER_BULLET_ID, true, 1024.0f,
        damage, charming));

    // right
    bullet_pool.push_back(std::make_unique<Cannon>(
        playerCenter + sf::Vector2f(horizontalOffset, verticalOffset),
        sf::Vector2f(0.0f, -1.0f), Constants::PLAYER_BULLET_ID, true, 1024.0f,
        damage, charming));

    static size_t counter = 0ul;
    if (shotSpeedIncreased) {
        if (counter % 2 == 0)
            ResourceManager::playSound("assets/bullet3.wav");
        for (int i = 0; i < 6; i++) {
            sf::Vector2f shootDirection =
                sf::Vector2f(RandomUtils::generateInRange(-0.4f, 0.4f), -1.0f);
            shootDirection = Math::normalize(shootDirection);
            bullet_pool.push_back(std::make_unique<Cannon>(
                playerCenter + sf::Vector2f(0.0f, verticalOffset * 2.0f),
                shootDirection, Constants::PLAYER_SUPER_BULLET_ID, true,
                1600.0f, damage, charming));
        }
    }
    counter++;

    lastShotTimer.restart();
}

void Player::takeDamage(float rawDamage) {
    float dmgMul = 1.0f;
    for (auto &gift : gifts)
        dmgMul *= (1.0f - gift->damageReduction);

    float finalDamage = rawDamage * dmgMul;
    finalDamage = std::max(0.0f, finalDamage);

    health -= finalDamage;
}

boost::json::object Player::serialize() const {
    boost::json::object o = Entity::serialize();
    o["current_shot_gap"] = current_shot_gap;
    o["health"] = health;
    o["damage"] = damage;
    o["recover_health"] = recover_health;
    return o;
}

void Player::deserialize(const boost::json::object &o) {
    Entity::deserialize(o);
    current_shot_gap = (float)o.at("current_shot_gap").as_double();
    health = (float)o.at("health").as_double();
    damage = (float)o.at("damage").as_double();
    recover_health = (float)o.at("recover_health").as_double();
}
