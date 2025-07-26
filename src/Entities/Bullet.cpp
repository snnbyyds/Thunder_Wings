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

Bullet::Bullet(sf::Vector2f position, sf::Vector2f direction, size_t id,
               bool from_player, float speed, float damage)
    : from_player(from_player), damage(damage), damageRate(0.0f),
      exploding(false), speed(speed), id(id) {
    avail = true;
    size_t bullets_path_size = sizeof(bullets_path) / sizeof(*bullets_path);
    id = std::min(id, bullets_path_size - 1);
    sprite.setTexture(ResourceManager::getTexture(bullets_path[id]));
    sprite.setPosition(position);
    this->direction = Math::normalize(direction);
    timer.restart();
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

void Bullet::explodeSoundOnly() {}

boost::json::object Bullet::serialize() const {
    boost::json::object o = Entity::serialize();
    o["from_player"] = from_player;
    o["damage"] = damage;
    o["damageRate"] = damageRate;
    o["time"] = timer.getElapsedTime();
    o["avail"] = avail && !exploding;
    o["direction"] = {{"x", direction.x}, {"y", direction.y}};
    o["speed"] = speed;
    o["id"] = id;
    return o;
}

void Bullet::deserialize(const boost::json::object &o) {
    Entity::deserialize(o);
    from_player = o.at("from_player").as_bool();
    damage = (float)o.at("damage").as_double();
    damageRate = (float)o.at("damageRate").as_double();
    timer.setElapsedTime((float)o.at("time").as_double());

    auto dir_obj = o.at("direction").as_object();
    direction.x = (float)dir_obj.at("x").as_double();
    direction.y = (float)dir_obj.at("y").as_double();

    speed = (float)o.at("speed").as_double();
    id = (size_t)o.at("id").as_int64();
}

// Cannon

Cannon::Cannon(const boost::json::object &o) {
    deserialize(o);
    sprite.setTexture(ResourceManager::getTexture(bullets_path[id]));
    sprite.setOrigin(sprite.getLocalBounds().width / 2,
                     sprite.getLocalBounds().height / 2);
}

Cannon::Cannon(sf::Vector2f position, sf::Vector2f direction, size_t id,
               bool from_player, float speed, float damage, bool charming)
    : Bullet(position, direction, id, from_player, speed, damage) {
    this->charming = charming;
    sprite.setOrigin(sprite.getLocalBounds().width / 2,
                     sprite.getLocalBounds().height / 2);
}

boost::json::object Cannon::serialize() const {
    boost::json::object o = Bullet::serialize();
    o["type"] = "Cannon";
    return o;
}

void Cannon::deserialize(const boost::json::object &o) {
    Bullet::deserialize(o);
}

// Missile

Missile::Missile(const boost::json::object &o) {
    deserialize(o);
    if (from_player)
        this->tracking = 0.0f;

    sprite.setTexture(ResourceManager::getTexture(bullets_path[id]));
    sprite.setOrigin(sprite.getLocalBounds().width / 2,
                     sprite.getLocalBounds().height);

    whiteFlash.setSize(
        sf::Vector2f(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT));
    whiteFlash.setFillColor(sf::Color(255, 255, 255, 220));

    if (id == Constants::ENEMY_MISSILE_ID)
        updateRotation();
}

Missile::Missile(sf::Vector2f position, sf::Vector2f direction, size_t id,
                 bool from_player, float speed, float damage, float tracking)
    : Bullet(position, direction, id, from_player, speed, damage),
      tracking(tracking) {

    if (from_player)
        this->tracking = 0.0f;

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
        speed += deltaTime * 100.0f;
        tracking += deltaTime * 0.00002f;
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
        if (timer.hasElapsed(2.5f))
            tracking = 0.0f;
    } else {
        speed += deltaTime * 220.0f;
    }

    sprite.move(direction * speed * deltaTime);

    auto [x, y] = sprite.getPosition();
    avail = (x >= 0 && x <= Constants::SCREEN_WIDTH && y >= 0 &&
             y <= Constants::SCREEN_HEIGHT);
}

void Missile::render(sf::RenderWindow &window) {
    if (exploding) {
        avail = false;
        if (explodeTimer.hasElapsed(0.6f)) {
            exploding = false;
        } else {
            if (!explodeTimer.hasElapsed(0.3f))
                window.draw(whiteFlash);
            window.draw(explodeSprite);
        }
    }
    if (avail)
        window.draw(sprite);
}

void Missile::explode() {
    explodeSoundOnly();
    explodeSprite.setTexture(ResourceManager::getTexture("assets/explode.png"));
    exploding = true;
    explodeTimer.restart();
}

void Missile::explodeSoundOnly() {
    ResourceManager::playSound("assets/explode.wav");
}

boost::json::object Missile::serialize() const {
    boost::json::object o = Bullet::serialize();
    o["type"] = "Missile";
    o["tracking"] = tracking;
    return o;
}

void Missile::deserialize(const boost::json::object &o) {
    Bullet::deserialize(o);
    tracking = (float)o.at("tracking").as_double();
}

// Rocket

Rocket::Rocket(const boost::json::object &o) {
    deserialize(o);
    sprite.setTexture(ResourceManager::getTexture(bullets_path[id]));
    sprite.setOrigin(sprite.getLocalBounds().width / 2,
                     sprite.getLocalBounds().height);

    redFlash.setSize(
        sf::Vector2f(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT));
    redFlash.setFillColor(sf::Color(255, 69, 1, 128));

    updateRotation();
}

Rocket::Rocket(sf::Vector2f position, sf::Vector2f direction, size_t id,
               bool from_player, float speed, float damage)
    : Bullet(position, direction, id, from_player, speed, damage) {

    damageRate = Constants::ROCKET_DAMAGE_RATE_INITIAL;

    if (from_player)
        this->tracking = 0.0f;

    sprite.setOrigin(sprite.getLocalBounds().width / 2,
                     sprite.getLocalBounds().height);

    redFlash.setSize(
        sf::Vector2f(Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT));
    redFlash.setFillColor(sf::Color(255, 69, 1, 128));

    updateRotation();
}

void Rocket::update(float deltaTime, sf::Vector2f hitTarget) {
    if (!avail)
        return;

    if (from_player)
        tracking = 0.0f;
    else
        tracking = 16384.0f;

    if (tracking != 0.0f && timer.hasElapsed(0.92f))
        tracking = 0.0f;

    if (damageRate != Constants::ROCKET_DAMAGE_RATE && timer.hasElapsed(0.04f))
        damageRate = Constants::ROCKET_DAMAGE_RATE;

    if (tracking > 0.0f) {
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
    }

    speed += deltaTime * 540.0f;

    sprite.move(direction * speed * deltaTime);

    auto [x, y] = sprite.getPosition();
    avail = (x >= 0 && x <= Constants::SCREEN_WIDTH && y >= 0 &&
             y <= Constants::SCREEN_HEIGHT);
}

void Rocket::render(sf::RenderWindow &window) {
    if (exploding) {
        avail = false;
        if (explodeTimer.hasElapsed(0.6f)) {
            exploding = false;
        } else {
            if (!explodeTimer.hasElapsed(0.3f))
                window.draw(redFlash);
            window.draw(explodeSprite);
        }
    }
    if (avail)
        window.draw(sprite);
}

void Rocket::explode() {
    explodeSoundOnly();
    explodeSprite.setTexture(
        ResourceManager::getTexture("assets/explode2.png"));
    exploding = true;
    explodeTimer.restart();
}

void Rocket::explodeSoundOnly() {
    ResourceManager::playSound("assets/explode.wav");
}

boost::json::object Rocket::serialize() const {
    boost::json::object o = Bullet::serialize();
    o["type"] = "Rocket";
    o["tracking"] = tracking;
    return o;
}

void Rocket::deserialize(const boost::json::object &o) {
    Bullet::deserialize(o);
    tracking = (float)o.at("tracking").as_double();
}
