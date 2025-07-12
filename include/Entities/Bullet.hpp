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
    Bullet(sf::Vector2f position, size_t id, bool from_player, float speed,
           float damage);
    Bullet(sf::Vector2f position, sf::Vector2f direction, size_t id,
           bool from_player, float speed, float damage);
    Bullet(sf::Vector2f position, sf::Vector2f direction, size_t id,
           bool from_player, float speed, float damage, float tracking);

    void update(float deltaTime, sf::Vector2f hitTarget);

    bool from_player;
    float damage;
    float tracking;
    Timer timer;

private:
    void updateRotation();

    const char *bullets_path[3] = {"assets/bullet1.png", "assets/bullet2.png",
                                   "assets/missle.png"};
    sf::Vector2f direction;
    float speed;
};