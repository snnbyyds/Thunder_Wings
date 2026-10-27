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

#include "thunder/core/Random.hpp"
#include "thunder/game/Types.hpp"

#include <cstdint>
#include <vector>

namespace thunder {

struct GiftState {
    GiftKind kind{};
    float remaining{};
    float maxTime{};
    bool warning1Played{};
    bool warning2Played{};
    float summonTimer{};
    int summonsRemaining{};
};

struct PlayerState {
    Vec2 position{};
    float health{};
    float shotCooldown{};
    float regenCooldown{};
    float animationTimer{};
    int textureIndex{};
    bool dying{};
    int deathFrame{};
    float deathTimer{};
    bool dead{};
    std::uint32_t shotCount{};
    std::vector<GiftState> gifts;
};

struct BulletState {
    BulletKind kind{};
    BulletVisual visual{};
    bool fromPlayer{};
    Vec2 position{};
    Vec2 direction{};
    float speed{};
    float damage{};
    float damageRate{};
    float tracking{};
    float age{};
};

struct EnemyState {
    EnemyType type{};
    Vec2 position{};
    float health{};
    float maxHealth{};
    float speed{};
    float bulletSpeed{};
    float shotDamage{};
    float killBonus{};
    float shotCooldown{};
    float shotGap{};
    int shotIndex{};
    float motionTime{};
    float motionCenterX{};
    float motionAmplitude{};
    float motionFrequency{};
    float hitFlash{};
    bool dying{};
    int deathFrame{};
    float deathTimer{};
    bool rewarded{};
    float recoverRate{};
};

struct AllyState {
    Vec2 position{};
    float health{};
    float fireCooldown{};
};

/// Everything required to deterministically resume a game. Visual-only effects
/// are intentionally omitted.
struct WorldState {
    RandomState random{};
    float elapsed{};
    std::uint64_t kills{};
    std::uint64_t alliesSummoned{};
    float enemySpawnTimer{};
    float giftSpawnTimer{};
    PlayerState player{};
    std::vector<EnemyState> enemies;
    std::vector<BulletState> bullets;
    std::vector<AllyState> allies;
};

} // namespace thunder
