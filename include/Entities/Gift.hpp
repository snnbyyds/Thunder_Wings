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

class Gift : public Entity {
public:
    Gift() = default;
    Gift(const boost::json::object &o);
    Gift(const std::string &name);
    virtual ~Gift() = default;

    void update(float deltaTime) override;
    float getRemainingTime() const;
    sf::Sprite &getSprite();

    boost::json::object serialize() const override;
    void deserialize(const boost::json::object &o) override;

    float damageReduction = 0.0f;
    float attackSpeedIncrease = 0.0f;
    bool charming = false;

protected:
    std::string name;
    float remainingTime;

private:
    Timer disappearingTimer;
    bool disappearing = false;
    bool disappearingSound1Played = false;
    bool disappearingSound2Played = false;
};

class FullFirePower : public Gift {
public:
    FullFirePower();

    FullFirePower(const boost::json::object &o) : Gift(o) {}
};

class CenturyShield : public Gift {
public:
    CenturyShield();

    CenturyShield(const boost::json::object &o) : Gift(o) {}
};

class AllMyPeople : public Gift {
public:
    AllMyPeople();

    AllMyPeople(const boost::json::object &o) : Gift(o) {}
};
