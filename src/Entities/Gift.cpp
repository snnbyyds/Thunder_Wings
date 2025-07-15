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
#include "Core/Constants.hpp"
#include "Core/RandomUtils.hpp"
#include "Core/ResourceManager.hpp"

Gift::Gift(const std::string &name) : name(name) {
    avail = true;
    remainingTime = (float)RandomUtils::generateInRange(7, 10);
    sprite.setTexture(ResourceManager::getTexture("assets/" + name + ".png"));

    constexpr float padding = 10.0f;
    constexpr float iconSize = 64.0f;
    constexpr float raiseUp = 32.0f;
    float x = padding;
    float y = Constants::SCREEN_HEIGHT - iconSize - padding - raiseUp;
    const auto bounds = sprite.getLocalBounds();
    float scale = iconSize / bounds.height;
    sprite.setScale(scale, scale);
    sprite.setPosition(x, y);

    ResourceManager::playSound("assets/" + name + ".wav");
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
        } else if (!disappearingSound2Played && disappearingTimer.hasElapsed(1.72f)) {
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

FullFirePower::FullFirePower() : Gift("FullFirePower") {
    attackSpeedIncrease = 4.0f; // +400%
    damageReduction = 0.0f;
}

CenturyShield::CenturyShield() : Gift("CenturyShield") {
    attackSpeedIncrease = 0.0f;
    damageReduction = 0.9f; // -90%
}
