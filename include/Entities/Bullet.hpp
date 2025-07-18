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

#pragma once
#include "../Core/Timer.hpp"
#include "Entity.hpp"
#include <SFML/Graphics.hpp>

class Bullet : public Entity {
public:
    Bullet() = default;
    Bullet(sf::Vector2f position, size_t id, bool from_player, float speed,
           float damage);
    Bullet(sf::Vector2f position, sf::Vector2f direction, size_t id,
           bool from_player, float speed, float damage);

    virtual void update(float deltaTime, sf::Vector2f hitTarget);
    virtual void render(sf::RenderWindow &window) override;
    virtual void explode();

    virtual boost::json::object serialize() const override;
    virtual void deserialize(const boost::json::object &o) override;

    bool from_player;
    float damage;
    float damageRate;
    Timer timer;
    bool exploding;

protected:
    void updateRotation();

    const char *bullets_path[4] = {"assets/bullet1.png", "assets/bullet2.png",
                                   "assets/missle.png", "assets/rocket.png"};
    sf::Vector2f direction;
    float speed;
    size_t id;

private:
    Bullet(const Bullet &) = delete;
    Bullet &operator=(const Bullet &) = delete;
};

class Cannon : public Bullet {
public:
    Cannon(const boost::json::object &o);
    Cannon(sf::Vector2f position, size_t id, bool from_player, float speed,
           float damage);
    Cannon(sf::Vector2f position, sf::Vector2f direction, size_t id,
           bool from_player, float speed, float damage);

    boost::json::object serialize() const override;
    void deserialize(const boost::json::object &o) override;
};

class Missile : public Bullet {
public:
    Missile(const boost::json::object &o);
    Missile(sf::Vector2f position, sf::Vector2f direction, size_t id,
            bool from_player, float speed, float damage, float tracking);

    void update(float deltaTime, sf::Vector2f hitTarget) override;
    void render(sf::RenderWindow &window) override;
    void explode() override;

    boost::json::object serialize() const override;
    void deserialize(const boost::json::object &o) override;

private:
    float tracking;
    sf::RectangleShape whiteFlash;
    Timer explodeTimer;
};

class Rocket : public Bullet {
public:
    Rocket(const boost::json::object &o);
    Rocket(sf::Vector2f position, sf::Vector2f direction, size_t id,
           bool from_player, float speed, float damage);

    void update(float deltaTime, sf::Vector2f hitTarget) override;
    void render(sf::RenderWindow &window) override;
    void explode() override;

    boost::json::object serialize() const override;
    void deserialize(const boost::json::object &o) override;

private:
    float tracking;
    sf::RectangleShape redFlash;
    Timer explodeTimer;
};
