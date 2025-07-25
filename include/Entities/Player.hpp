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
#include "../Core/Constants.hpp"
#include "../Core/Timer.hpp"
#include "Bullet.hpp"
#include "Entities/Gift.hpp"
#include "Entity.hpp"
#include <SFML/Graphics.hpp>
#include <array>
#include <memory>
#include <string>
#include <vector>

class Player : public Entity {
public:
    Player();

    void update(float deltaTime) override;
    void render(sf::RenderWindow &window) override;

    void move(float deltaTime);
    void updateCollisions(std::vector<std::unique_ptr<Bullet>> &bullet_pool);
    void shoot(std::vector<std::unique_ptr<Bullet>> &bullet_pool);
    void takeDamage(float rawDamage);

    boost::json::object serialize() const override;
    void deserialize(const boost::json::object &o) override;

    const float max_health = Constants::PLAYER_MAX_HEALTH;
    bool dying;
    bool charming = false;
    bool hasShield = false;
    float current_shot_gap;
    float health;
    float damage;
    float recover_health;
    std::vector<std::unique_ptr<Gift>> gifts;

private:
    const std::array<std::string, 2> images = {"assets/me1.png",
                                               "assets/me2.png"};
    const std::array<std::string, 4> deathImages = {
        "assets/me_destroy_1.png", "assets/me_destroy_2.png",
        "assets/me_destroy_3.png", "assets/me_destroy_4.png"};

    float speed;
    size_t current_texture;
    Timer lastShotTimer;
    Timer deathTimer;
    Timer animationTimer;
    Timer recoverTimer;
    size_t destroyFrameIdx;

    sf::Sprite shieldSprite;
};