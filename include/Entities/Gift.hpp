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
#include "Player.hpp"
#include <SFML/Graphics.hpp>

class Gift : public Entity {
public:
    Gift() = default;
    Gift(const std::string &name);

    std::string getName() const { return name; }
    bool isApplied() const { return applied; }
    void update(float deltaTime, Player &player);
    void move(float deltaTime);
    void render(sf::RenderWindow &window) override;

    void applyTo(Player &player);
    void removeFrom(Player &player);

protected:
    // Buffs
    float additionalRecovery;
    float damageReduction;
    bool superAttack; // Reduce player's attack gap to zero

private:
    void update(float deltaTime) override {};

    std::string name;
    bool applied;
    float speed;
    sf::Vector2f direction;

    Timer effectTimer;
};

class SuperRecovery : public Gift {
public:
    SuperRecovery();
};

class CenturyShield : public Gift {
public:
    CenturyShield();
};

class FullFirePower : public Gift {
public:
    FullFirePower();
};
