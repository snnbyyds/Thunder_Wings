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
#include "Core/Macros.h"
#include "Core/Math.hpp"
#include "Core/RandomUtils.hpp"
#include "Core/ResourceManager.hpp"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(int level, sf::Vector2f position)
    : level(level), downFrameIdx(1), dying(false) {
    avail = true;

    switch (level) {
        case 1:
            maxHealth = health = 2048.0f;
            speed = RandomUtils::generateInRange(128.0f, 256.0f);
            bulletspeed = speed * 3.2f;
            current_shot_gap = 0.2f;
            damage = 512.0f;
            killBonus = 5000.0f;
            break;
        case 2:
            maxHealth = health = 16384.0f;
            speed = RandomUtils::generateInRange(64.0f, 256.0f);
            bulletspeed = speed * 4.0f;
            current_shot_gap = 0.1f;
            damage = 728.0f;
            killBonus = 10000.0f;
            break;
        case 3:
            maxHealth = health =
                RandomUtils::generateInRange(840000.0f, 1200000.0f);
            speed = RandomUtils::generateInRange(16.0f, 32.0f);
            bulletspeed = RandomUtils::generateInRange(1024.0f, 2048.f);
            current_shot_gap = RandomUtils::generateInRange(0.2f, 0.28f);
            damage = 4096.0f;
            killBonus = maxHealth;
            break;
        default: __unreachable(); break;
    }

    sprite.setTexture(ResourceManager::getTexture(
        std::string("assets/enemy") + std::to_string(level) + ".png"));
    sprite.setPosition(position);
    sprite.setColor(sf::Color::Yellow);

    bonusTaken = false;
    charmed = false;
}

Enemy::Enemy(const boost::json::object &o) {
    deserialize(o);
    sprite.setTexture(ResourceManager::getTexture(
        std::string("assets/enemy") + std::to_string(level) + ".png"));
    sprite.setColor(sf::Color::Yellow);
}

void Enemy::update(float deltaTime) {
    if (!avail)
        return;

    if (charmed)
        sprite.setColor(sf::Color::Cyan);

    move(deltaTime);

    if (health <= 0.0f) {
        if (!dying) {
            dying = true;
            std::string dyingSound = std::string("assets/enemy") +
                                     std::to_string(level) + "_down.wav";
            ResourceManager::playSound(dyingSound);
        }

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
    } else {
        recover(deltaTime);
    }
}

void Enemy::move(float deltaTime) {
    if (!avail)
        return;

    sprite.setRotation(charmed ? 180.0f : 0.0f);
    if (charmed &&
        sprite.getPosition().y >= Constants::SCREEN_HEIGHT * 2.0f / 3.0f)
        speed = 0.0f;

    sprite.move(0, speed * deltaTime * (charmed ? -1 : 1));
    auto [x, y] = sprite.getPosition();
    avail = (x >= 0 && x < Constants::SCREEN_WIDTH && y >= 0 &&
             y <= Constants::SCREEN_HEIGHT);
}

void Enemy::shoot(std::vector<std::unique_ptr<Bullet>> &bullet_pool) {
    if (!avail || health <= 0.0f)
        return;

    if (current_shot_gap > 0.0f && !lastShotTimer.hasElapsed(current_shot_gap))
        return;

    const sf::FloatRect bounds = sprite.getGlobalBounds();
    sf::Vector2f spawnPosition(bounds.left + bounds.width / 2.0f,
                               bounds.top + bounds.height + 8.0f);
    sf::Vector2f direction = {0.0f, 1.0f};
    if (charmed) {
        spawnPosition = {bounds.left + bounds.width / 2.0f, bounds.top - 8.0f};
        direction = {0.0f, -1.0f};
    }
    bullet_pool.push_back(std::make_unique<Cannon>(
        spawnPosition, direction,
        charmed ? Constants::PLAYER_BULLET_ID : Constants::ENEMY_BULLET_ID,
        charmed, bulletspeed, damage, false));

    lastShotTimer.restart();
}

void Enemy::takeDamage(float damage) {
    health -= damage;
    sprite.setTexture(ResourceManager::getTexture(
        std::string("assets/enemy") + std::to_string(level) + "_hit.png"));
}

void Enemy::recover(float deltaTime) {
    if (avail && charmed)
        health = std::min(maxHealth * 15.0f, deltaTime * 240.0f + health);
}

boost::json::object Enemy::serialize() const {
    boost::json::object o = Entity::serialize();
    o["level"] = level;
    o["health"] = health;
    o["maxHealth"] = maxHealth;
    o["killBonus"] = killBonus;
    o["speed"] = speed;
    o["bulletspeed"] = bulletspeed;
    o["current_shot_gap"] = current_shot_gap;
    o["damage"] = damage;
    o["avail"] = avail && !dying;
    o["charmed"] = charmed;
    o["bonusTaken"] = bonusTaken;
    return o;
}

void Enemy::deserialize(const boost::json::object &o) {
    Entity::deserialize(o);
    level = (int)o.at("level").as_int64();
    health = (float)o.at("health").as_double();
    maxHealth = (float)o.at("maxHealth").as_double();
    killBonus = (float)o.at("killBonus").as_double();
    speed = (float)o.at("speed").as_double();
    bulletspeed = (float)o.at("bulletspeed").as_double();
    current_shot_gap = (float)o.at("current_shot_gap").as_double();
    damage = (float)o.at("damage").as_double();
    charmed = o.at("charmed").as_bool();
    bonusTaken = o.at("bonusTaken").as_bool();
}

void Enemy::updateBulletCollisions(
    std::vector<std::unique_ptr<Bullet>> &bullet_pool) {
    if (!avail || health <= 0.0f)
        return;

    const auto bounds = getBounds();
    for (auto &bullet : bullet_pool) {
        if (!bullet->isAvailable() || !bounds.intersects(bullet->getBounds()))
            continue;
        if (!charmed && bullet->charming && level < 3) {
            charmed = true;
            speed /= -2.0f;
            health *= 10.0f;
            damage *= 1.6f;
            bullet->setAvailable(false);
            ResourceManager::playSound("assets/AllMyPeople.wav");
        } else if ((!bullet->from_player && charmed) ||
                   (bullet->from_player && !charmed)) {
            takeDamage(std::max(bullet->damage, bullet->damageRate * health));
            bullet->explodeSoundOnly();
            bullet->setAvailable(false);
        }
    }
}

/* Enemy1 Implementation */

Enemy1::Enemy1(const boost::json::object &o) : Enemy(o) { deserialize(o); }

Enemy1::Enemy1(sf::Vector2f position) : Enemy(1, position) {}

boost::json::object Enemy1::serialize() const { return Enemy::serialize(); }

void Enemy1::deserialize(const boost::json::object &o) {
    Enemy::deserialize(o);
}

/* Enemy2 Implementation */

Enemy2::Enemy2(const boost::json::object &o) : Enemy(o) { deserialize(o); }

Enemy2::Enemy2(sf::Vector2f position) : Enemy(2, position) {
    verticalAmplitude = RandomUtils::generateInRange(128.0f, 256.0f);
    verticalFrequency = RandomUtils::generateInRange(0.5f, 1.5f);
    verticalCenter = position.x;
    timer.restart();
}

void Enemy2::move(float deltaTime) {
    if (!avail)
        return;

    sprite.setRotation(charmed ? 180.0f : 0.0f);
    if (charmed &&
        sprite.getPosition().y >= Constants::SCREEN_HEIGHT * 4.0f / 5.0f)
        speed = 0.0f;

    float verticalOffset =
        std::sin(timer.getElapsedTime() * verticalFrequency) *
        verticalAmplitude;
    sprite.move(0, speed * deltaTime * (charmed ? -1 : 1));
    auto [x, y] = sprite.getPosition();
    sprite.setPosition(verticalCenter + verticalOffset, y);
    avail = (y >= 0 && y <= Constants::SCREEN_HEIGHT);
}

boost::json::object Enemy2::serialize() const {
    boost::json::object o = Enemy::serialize();
    o["verticalAmplitude"] = verticalAmplitude;
    o["verticalFrequency"] = verticalFrequency;
    o["verticalCenter"] = verticalCenter;
    o["time"] = timer.getElapsedTime();
    return o;
}

void Enemy2::deserialize(const boost::json::object &o) {
    Enemy::deserialize(o);
    verticalAmplitude = (float)o.at("verticalAmplitude").as_double();
    verticalFrequency = (float)o.at("verticalFrequency").as_double();
    verticalCenter = (float)o.at("verticalCenter").as_double();
    timer.setElapsedTime((float)o.at("time").as_double());
}

/* Enemy3 Implementation */

Enemy3::Enemy3(const boost::json::object &o) : Enemy(o) { deserialize(o); }

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

void Enemy3::shoot(std::vector<std::unique_ptr<Bullet>> &bullet_pool) {
    if (!avail || health <= 0.0f)
        return;

    if (current_shot_gap > 0.0f && !lastShotTimer.hasElapsed(current_shot_gap))
        return;

    static size_t shootCounter = 0;

    const sf::FloatRect bounds = sprite.getGlobalBounds();
    const float centerX = bounds.left + bounds.width / 2.0f;
    const float bottomY = bounds.top + bounds.height + 8.0f;

    for (int i = 0; i < 6; i++) {
        sf::Vector2f shootDirection =
            sf::Vector2f(RandomUtils::generateInRange(-0.32f, 0.32f), 1.0f);
        shootDirection = Math::normalize(shootDirection);
        bullet_pool.push_back(std::make_unique<Cannon>(
            sf::Vector2f(centerX, bottomY), shootDirection,
            Constants::ENEMY3_BULLET_ID, false, bulletspeed, damage, false));
    }

    shootCounter = (shootCounter + 1) % 16;
    if (shootCounter == 0 || (health < 12480.0f)) {
        const int missileCount = health < maxHealth * 0.4f ? 4 : 2;
        for (int i = 0; i < missileCount; i++) {
            bullet_pool.push_back(std::make_unique<Missile>(
                sf::Vector2f(centerX - 50.0f - i * 20.0f, bottomY - i * 36.0f),
                sf::Vector2f(0.0f, 1.0f), Constants::ENEMY_MISSILE_ID, false,
                bulletspeed * (0.08f + i * 0.01f), damage * 4.2f,
                0.4f + i * 0.5f));

            bullet_pool.push_back(std::make_unique<Missile>(
                sf::Vector2f(centerX + 50.0f + i * 20.0f, bottomY - i * 36.0f),
                sf::Vector2f(0.0f, 1.0f), Constants::ENEMY_MISSILE_ID, false,
                bulletspeed * (0.02f + i * 0.03f), damage * 4.2f,
                0.4f + i * 0.5f));
        }

        ResourceManager::playSound("assets/missile.wav");
    } else if (shootCounter == 4 || shootCounter == 6 ||
               (health < maxHealth * 0.32f && shootCounter == 9)) {
        bullet_pool.push_back(std::make_unique<Rocket>(
            sf::Vector2f(centerX - 50.0f, bottomY), sf::Vector2f(0.0f, 1.0f),
            Constants::ENEMY_ROCKET_ID, false, bulletspeed * 0.01f,
            damage * 1.6f));

        bullet_pool.push_back(std::make_unique<Rocket>(
            sf::Vector2f(centerX + 50.0f, bottomY), sf::Vector2f(0.0f, 1.0f),
            Constants::ENEMY_ROCKET_ID, false, bulletspeed * 0.14f,
            damage * 1.6f));

        ResourceManager::playSound("assets/rocket.wav");
    } else {
        ResourceManager::playSound("assets/bullet.wav");
    }

    lastShotTimer.restart();
}

void Enemy3::recover(float deltaTime) {
    health = std::min(health + deltaTime * recoverRate, maxHealth * 2.0f);
}

boost::json::object Enemy3::serialize() const {
    boost::json::object o = Enemy::serialize();
    o["verticalAmplitude"] = verticalAmplitude;
    o["verticalFrequency"] = verticalFrequency;
    o["verticalCenter"] = verticalCenter;
    o["recoverRate"] = recoverRate;
    o["time"] = timer.getElapsedTime();
    return o;
}

void Enemy3::deserialize(const boost::json::object &o) {
    Enemy::deserialize(o);
    verticalAmplitude = (float)o.at("verticalAmplitude").as_double();
    verticalFrequency = (float)o.at("verticalFrequency").as_double();
    verticalCenter = (float)o.at("verticalCenter").as_double();
    recoverRate = (float)o.at("recoverRate").as_double();
    timer.setElapsedTime((float)o.at("time").as_double());
}
