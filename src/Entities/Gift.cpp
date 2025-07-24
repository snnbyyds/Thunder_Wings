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

#include "Entities/Gift.hpp"
#include "Core/RandomUtils.hpp"
#include "Core/ResourceManager.hpp"

Gift::Gift(const boost::json::object &o) {
    deserialize(o);
    sprite.setTexture(ResourceManager::getTexture("assets/" + name + ".png"));
    constexpr float iconSize = 80.0f;
    auto bounds = sprite.getLocalBounds();
    float scale = iconSize / bounds.height;
    sprite.setScale(scale, scale);
}

Gift::Gift(const std::string &name) : name(name) {
    avail = true;
    remainingTime = (float)RandomUtils::generateInRange(7, 10);
    if (name == "AllMyPeople")
        remainingTime += 4.0f; // Extra time for AllMyPeople
    maxTime = remainingTime;
    sprite.setTexture(ResourceManager::getTexture("assets/" + name + ".png"));
    ResourceManager::playSound("assets/" + name + ".wav");
    constexpr float iconSize = 80.0f;
    auto bounds = sprite.getLocalBounds();
    float scale = iconSize / bounds.height;
    sprite.setScale(scale, scale);
}

void Gift::update(float deltaTime) {
    if (!avail)
        return;
    remainingTime -= deltaTime;
    if (!disappearing && remainingTime <= 3.5f) {
        disappearingTimer.restart();
        disappearing = true;
    }
    if (disappearing) {
        if (!disappearingSound1Played && disappearingTimer.hasElapsed(0.72f)) {
            ResourceManager::playSound("assets/gift_disappear1.wav");
            disappearingSound1Played = true;
        } else if (!disappearingSound2Played &&
                   disappearingTimer.hasElapsed(1.72f)) {
            ResourceManager::playSound("assets/gift_disappear1.wav");
            disappearingSound2Played = true;
        }
    }
    if (remainingTime <= 0.0f) {
        avail = false;
        ResourceManager::playSound("assets/gift_disappear2.wav");
    }
}

float Gift::getRemainingTime() const { return remainingTime; }

sf::Sprite &Gift::getSprite() { return sprite; }

boost::json::object Gift::serialize() const {
    boost::json::object o = Entity::serialize();
    o["damageReduction"] = damageReduction;
    o["attackSpeedIncrease"] = attackSpeedIncrease;
    o["charming"] = charming;
    o["name"] = name;
    o["remainingTime"] = remainingTime;
    o["maxTime"] = maxTime;
    o["disappearingTime"] = disappearingTimer.getElapsedTime();
    o["disappearing"] = disappearing;
    o["disappearingSound1Played"] = disappearingSound1Played;
    o["disappearingSound2Played"] = disappearingSound2Played;
    return o;
}

void Gift::deserialize(const boost::json::object &o) {
    Entity::deserialize(o);
    damageReduction = (float)o.at("damageReduction").as_double();
    attackSpeedIncrease = (float)o.at("attackSpeedIncrease").as_double();
    charming = o.at("charming").as_bool();
    name = o.at("name").as_string();
    remainingTime = (float)o.at("remainingTime").as_double();
    maxTime = (float)o.at("maxTime").as_double();
    disappearingTimer.setElapsedTime(
        (float)o.at("disappearingTime").as_double());
    disappearing = o.at("disappearing").as_bool();
    disappearingSound1Played = o.at("disappearingSound1Played").as_bool();
    disappearingSound2Played = o.at("disappearingSound2Played").as_bool();
}

FullFirePower::FullFirePower() : Gift("FullFirePower") {
    attackSpeedIncrease = 4.0f; // +400%
    damageReduction = 0.0f;
    charming = false;
}

CenturyShield::CenturyShield() : Gift("CenturyShield") {
    attackSpeedIncrease = 0.0f;
    damageReduction = 0.9f; // -90%
    charming = false;
}

AllMyPeople::AllMyPeople() : Gift("AllMyPeople") {
    attackSpeedIncrease = 0.0f;
    damageReduction = 0.0f;
    charming = true;
}
