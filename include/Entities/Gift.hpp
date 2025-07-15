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
#include "Entity.hpp"

class Gift : public Entity {
public:
    Gift() = default;
    Gift(const std::string &name);
    virtual ~Gift() = default;

    void update(float deltaTime) override;
    float getRemainingTime() const;
    sf::Sprite &getSprite();

    float damageReduction;
    float attackSpeedIncrease;

protected:
    std::string name;
    float remainingTime;
};

class FullFirePower : public Gift {
public:
    FullFirePower();
};

class CenturyShield : public Gift {
public:
    CenturyShield();
};
