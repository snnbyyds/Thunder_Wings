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

#include "thunder/core/Math.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace thunder {

enum class EnemyType : std::uint8_t {
    Scout = 1,
    Sine = 2,
    Boss = 3,
};

enum class BulletKind : std::uint8_t {
    Cannon,
    Missile,
    Rocket,
};

/// Texture IDs are kept independent from the behaviour kind so the saved format
/// does not depend on asset ordering.
enum class BulletVisual : std::uint8_t {
    EnemyCannon,
    PlayerCannon,
    Missile,
    Rocket,
    PlayerSuper,
    BossBullet,
};

enum class GiftKind : std::uint8_t {
    FullFirePower,
    CenturyShield,
    AllMyPeople,
};

enum class ExplosionKind : std::uint8_t {
    Missile,
    Rocket,
};

enum class SoundId : std::uint8_t {
    Pause,
    Bullet,
    Bullet3,
    Missile,
    Rocket,
    Explode,
    Enemy1Down,
    Enemy2Down,
    Enemy3Down,
    PlayerDown,
    GiftDisappear1,
    GiftDisappear2,
    FullFirePower,
    CenturyShield,
    AllMyPeople,
    Count,
};

struct PlayerInput {
    bool left{};
    bool right{};
    bool up{};
    bool down{};
};

struct Gift {
    GiftKind kind{};
    float remaining{};
    float maxTime{};
    bool warning1Played{};
    bool warning2Played{};
    float summonTimer{};
    int summonsRemaining{};
};

struct Player {
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
    std::vector<Gift> gifts;
};

struct Bullet {
    BulletKind kind{};
    BulletVisual visual{};
    bool fromPlayer{};
    Vec2 position{};
    Vec2 direction{0.0f, 1.0f};
    float speed{};
    float damage{};
    float damageRate{};
    float tracking{};
    float age{};
};

struct Enemy {
    EnemyType type{};
    Vec2 position{}; // top-left, matching the original sprite placement
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

struct Ally {
    Vec2 position{};
    float health{};
    float fireCooldown{};
};

struct Explosion {
    ExplosionKind kind{};
    float timer{};
};

constexpr std::string_view enemyTypeName(EnemyType type) {
    switch (type) {
        case EnemyType::Scout: return "Scout";
        case EnemyType::Sine: return "Sine";
        case EnemyType::Boss: return "Boss";
    }
    return "Scout";
}

constexpr std::string_view bulletKindName(BulletKind kind) {
    switch (kind) {
        case BulletKind::Cannon: return "Cannon";
        case BulletKind::Missile: return "Missile";
        case BulletKind::Rocket: return "Rocket";
    }
    return "Cannon";
}

constexpr std::string_view bulletVisualName(BulletVisual visual) {
    switch (visual) {
        case BulletVisual::EnemyCannon: return "EnemyCannon";
        case BulletVisual::PlayerCannon: return "PlayerCannon";
        case BulletVisual::Missile: return "Missile";
        case BulletVisual::Rocket: return "Rocket";
        case BulletVisual::PlayerSuper: return "PlayerSuper";
        case BulletVisual::BossBullet: return "BossBullet";
    }
    return "EnemyCannon";
}

constexpr std::string_view giftKindName(GiftKind kind) {
    switch (kind) {
        case GiftKind::FullFirePower: return "FullFirePower";
        case GiftKind::CenturyShield: return "CenturyShield";
        case GiftKind::AllMyPeople: return "AllMyPeople";
    }
    return "FullFirePower";
}

constexpr int enemyLevel(EnemyType type) {
    return static_cast<int>(type);
}

constexpr int enemyDeathFrameCount(EnemyType type) {
    return type == EnemyType::Boss ? 6 : 4;
}

constexpr float giftAttackSpeedIncrease(GiftKind kind) {
    return kind == GiftKind::FullFirePower ? 4.0f : 0.0f;
}

constexpr float giftDamageReduction(GiftKind kind) {
    return kind == GiftKind::CenturyShield ? 0.9f : 0.0f;
}

constexpr bool giftIsAllMyPeople(GiftKind kind) {
    return kind == GiftKind::AllMyPeople;
}

constexpr bool isBoss(const Enemy &enemy) {
    return enemy.type == EnemyType::Boss;
}

} // namespace thunder
