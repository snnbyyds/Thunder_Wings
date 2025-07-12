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
#include "Core/Math.hpp"
#include "Core/RandomUtils.hpp"
#include "Core/ResourceManager.hpp"
#include <algorithm>

Gift::Gift(const std::string &name) : Entity(sf::Vector2f(Constants::SCREEN_WIDTH / 2.0f, Constants::SCREEN_HEIGHT * 3.0f / 4.0f)) {
    this->name = name;
    this->applied = false;
    setAvailable(true);

    float angleDegrees = RandomUtils::generateInRange(-75.0f, 75.0f);
    float angleRadians = angleDegrees * Math::DEG_TO_RAD;
    direction = Math::normalize(Math::angleToVector(angleRadians));
    speed = RandomUtils::generateInRange(64.0f, 300.0f);

    sprite.setTexture(ResourceManager::getTexture("assets/" + name + ".png"));
}

void Gift::update(float deltaTime, Player &player) {
    if (!isAvailable())
        return;
    if (applied) {
        if (effectTimer.hasElapsed(Constants::GIFT_BUFF_TIME))
            removeFrom(player);
    } else {
        move(deltaTime);
    }
}

void Gift::move(float deltaTime) {
    if (!isAvailable() || applied)
        return;

    sprite.move(deltaTime * direction * speed);

    auto [x, y] = sprite.getPosition();
    bool avail = (x >= 0 && x < Constants::SCREEN_WIDTH && y >= 0 &&
             y <= Constants::SCREEN_HEIGHT);
    setAvailable(avail);
}

void Gift::render(sf::RenderWindow &window) {
    if (isAvailable() && !applied)
        window.draw(sprite);
}

void Gift::applyTo(Player &player) {
    player.additionalRecovery += additionalRecovery;
    player.damageReduction += damageReduction;
    player.superAttackCount += superAttack;
    applied = true;
    effectTimer.restart();
}

void Gift::removeFrom(Player &player) {
    player.additionalRecovery = std::max(0.0f, player.additionalRecovery - additionalRecovery);
    player.damageReduction = std::max(0.0f, player.damageReduction - damageReduction);
    player.superAttackCount = std::max(0, player.superAttackCount - superAttack);
    setAvailable(false);
}

SuperRecovery::SuperRecovery() : Gift("SuperRecovery") {
    additionalRecovery = 4096.0f;
    damageReduction = 0.0f;
    superAttack = false;
}

CenturyShield::CenturyShield() : Gift("CenturyShield") {
    additionalRecovery = 512.0f;
    damageReduction = 96.0f;
    superAttack = false;
}

FullFirePower::FullFirePower() : Gift("FullFirePower") {
    additionalRecovery = 0.0f;
    damageReduction = 0.0f;
    superAttack = true;
}
