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
#include "Bullet.hpp"
#include "Entity.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class Enemy : public Entity {
public:
    Enemy(int level, sf::Vector2f position);
    virtual ~Enemy() = default;

    void update(float deltaTime) override;

    virtual void move(float deltaTime);
    virtual void shoot(std::vector<std::unique_ptr<Bullet>> &bullet_pool);
    virtual void takeDamage(float damage);
    virtual void recover(float deltaTime);

    void
    updateBulletCollisions(std::vector<std::unique_ptr<Bullet>> &bullet_pool);

    float health;
    float maxHealth;
    float killBonus;
    int level;

protected:
    float speed;
    float bulletspeed;
    Timer lastCollideTimer;
    Timer lastShotTimer;
    Timer animationTimer;
    float current_shot_gap;
    float damage;
    size_t downFrameIdx;
    bool dying;
};

class Enemy1 : public Enemy {
public:
    Enemy1(sf::Vector2f position);
};

class Enemy2 : public Enemy {
public:
    Enemy2(sf::Vector2f position);

    void move(float deltaTime) override;

private:
    float verticalAmplitude; // Random amplitude for vertical movement
    float verticalFrequency; // Random frequency for the oscillation
    float verticalCenter;
    Timer timer;
};

class Enemy3 : public Enemy {
public:
    Enemy3(sf::Vector2f position);

    void move(float deltaTime) override;
    void shoot(std::vector<std::unique_ptr<Bullet>> &bullet_pool) override;
    void recover(float deltaTime) override;

private:
    float verticalAmplitude; // Random amplitude for vertical movement
    float verticalFrequency; // Random frequency for the oscillation
    float verticalCenter;
    float recoverRate = 512.0f;
    Timer timer;
};
