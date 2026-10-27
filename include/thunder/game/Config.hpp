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
#include "thunder/game/Types.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace thunder::config {

// Common screen and timing properties.
inline constexpr int kScreenWidth = 1440;
inline constexpr int kScreenHeight = 900;
inline constexpr float kGameOverDelay = 3.2f;
inline constexpr float kInstructionsDuration = 8.0f;
inline constexpr float kFixedTimeStep = 1.0f / 120.0f;
inline constexpr float kMaxFrameDelta = 0.25f;
inline constexpr unsigned int kFramerateLimit = 60;

// Player properties.
inline constexpr Vec2 kPlayerStartPosition{400.0f, 500.0f};
inline constexpr float kPlayerSpeed = 700.0f;
inline constexpr float kPlayerMaxHealth = 1'024'000.0f;
inline constexpr float kPlayerInitialHealthRatio = 0.64f;
inline constexpr float kPlayerDamage = 6'144.0f;
inline constexpr float kPlayerShotGap = 0.144f;
inline constexpr float kPlayerRecoverHealth = 128.0f;
inline constexpr float kPlayerRecoverInterval = 0.64f;
inline constexpr float kPlayerSpriteScale = 0.64f;
inline constexpr float kPlayerHorizontalShotOffset = 20.0f;
inline constexpr float kPlayerVerticalShotOffset = -16.0f;
inline constexpr float kPlayerAnimationInterval = 0.16f;
inline constexpr float kPlayerDeathFrameInterval = 0.4f;
inline constexpr int kPlayerDeathFrameCount = 4;
inline constexpr float kPlayerSuperBulletSpeed = 1'600.0f;
inline constexpr float kPlayerSuperBulletSpread = 0.4f;
inline constexpr int kPlayerSuperBulletCount = 6;
inline constexpr std::size_t kMaxActiveGifts = 3;

inline constexpr Vec2 kPlayerCollisionSize{102.0f * kPlayerSpriteScale,
                                           126.0f * kPlayerSpriteScale};
inline constexpr Vec2 kPlayerShieldCollisionSize{405.0f * kPlayerSpriteScale,
                                                 397.0f * kPlayerSpriteScale};

// Gift properties.
inline constexpr float kGiftSpawnInterval = 32.0f;
inline constexpr float kGiftSpawnProbability = 0.36f;
inline constexpr float kGiftTripleChance = kGiftSpawnProbability / 2.0f;
inline constexpr int kGiftDurationMin = 7;
inline constexpr int kGiftDurationMax = 10;
inline constexpr float kAllMyPeopleDuration = 16.0f;
inline constexpr int kAllMyPeopleSummons = 16;
inline constexpr int kAllMyPeoplePerWave = 16;
inline constexpr float kGiftWarningTime = 3.5f;
inline constexpr float kGiftWarningDelay1 = 0.72f;
inline constexpr float kGiftWarningDelay2 = 1.72f;
inline constexpr std::array<GiftKind, 3> kGiftKinds = {
    GiftKind::FullFirePower,
    GiftKind::CenturyShield,
    GiftKind::AllMyPeople,
};
// Gift weights retain the original relative values.
inline constexpr std::array<float, 3> kGiftWeights = {20.0f, 25.0f, 15.0f};

// Enemy properties.
inline constexpr float kEnemySpawnInterval = 0.6f;
inline constexpr float kBossSpawnInterval = 0.1f;
inline constexpr float kEnemy1SpawnProbability = 0.6f;
inline constexpr float kEnemy2SpawnProbability = 0.38f;
inline constexpr float kEnemy3SpawnProbability = 0.02f;
inline constexpr int kEnemy1MaxAlive = 32;
inline constexpr int kEnemy2MaxAlive = 32;
inline constexpr int kEnemy3MaxAlive = 1;
inline constexpr float kBossSpawnTime = 32.0f;
inline constexpr int kBossWaveScouts = 32;
inline constexpr int kBossWaveSine = 24;
inline constexpr std::array<float, 3> kEnemyLevelWeights = {
    kEnemy1SpawnProbability,
    kEnemy2SpawnProbability,
    kEnemy3SpawnProbability,
};

inline constexpr float kEnemy1Health = 2'048.0f;
inline constexpr float kEnemy2Health = 16'384.0f;
inline constexpr float kEnemy3HealthMin = 840'000.0f;
inline constexpr float kEnemy3HealthMax = 1'200'000.0f;
inline constexpr float kEnemy1SpeedMin = 128.0f;
inline constexpr float kEnemy1SpeedMax = 256.0f;
inline constexpr float kEnemy2SpeedMin = 64.0f;
inline constexpr float kEnemy2SpeedMax = 256.0f;
inline constexpr float kEnemy3SpeedMin = 16.0f;
inline constexpr float kEnemy3SpeedMax = 32.0f;
inline constexpr float kEnemy1BulletSpeedFactor = 3.2f;
inline constexpr float kEnemy2BulletSpeedFactor = 4.0f;
inline constexpr float kEnemy1ShotGap = 0.2f;
inline constexpr float kEnemy2ShotGap = 0.1f;
inline constexpr float kEnemy3ShotGapMin = 0.2f;
inline constexpr float kEnemy3ShotGapMax = 0.28f;
inline constexpr float kEnemy1Damage = 512.0f;
inline constexpr float kEnemy2Damage = 728.0f;
inline constexpr float kEnemy3Damage = 4'096.0f;
inline constexpr float kEnemy1KillBonus = 5'000.0f;
inline constexpr float kEnemy2KillBonus = 10'000.0f;
inline constexpr float kEnemy3RecoverRate = 1'000.0f;
inline constexpr float kEnemy3RecoverCapMultiplier = 2.0f;
inline constexpr float kEnemy3HomingSpeedMin = 1'024.0f;
inline constexpr float kEnemy3HomingSpeedMax = 2'048.0f;
inline constexpr float kEnemyBulletSpawnOffset = 8.0f;

inline constexpr std::array<Vec2, 3> kEnemyCollisionSizes = {
    Vec2{57.0f, 43.0f},
    Vec2{69.0f, 99.0f},
    Vec2{169.0f, 258.0f},
};

// Bullet properties.
inline constexpr float kRocketDamageRateInitial = 0.84f;
inline constexpr float kRocketDamageRate = 0.184f;
inline constexpr float kMissileAcceleration = 100.0f;
inline constexpr float kMissileTrackingAcceleration = 0.00002f;
inline constexpr float kMissileTrackingDuration = 4.2f;
inline constexpr float kMissileFallbackAcceleration = 220.0f;
inline constexpr float kRocketAcceleration = 540.0f;
inline constexpr float kRocketTrackingDuration = 0.92f;
inline constexpr float kRocketTracking = 16'384.0f;
inline constexpr float kExplosionDuration = 0.6f;
inline constexpr float kExplosionFlashDuration = 0.3f;

inline constexpr std::array<Vec2, 6> kBulletCollisionSizes = {
    Vec2{9.0f, 28.0f},  // EnemyCannon
    Vec2{9.0f, 30.0f},  // PlayerCannon
    Vec2{35.0f, 87.0f}, // Missile
    Vec2{20.0f, 74.0f}, // Rocket
    Vec2{32.0f, 64.0f}, // PlayerSuper
    Vec2{32.0f, 64.0f}, // BossBullet
};

// Boss pattern thresholds preserved from the original game.
inline constexpr float kBossMissileHealthThreshold = 12'480.0f;
inline constexpr float kBossMissileLowHealthRatio = 0.4f;
inline constexpr float kBossRocketLowHealthRatio = 0.32f;
inline constexpr int kBossPatternCycle = 16;
inline constexpr int kBossMissileIndex = 0;
inline constexpr int kBossRocketIndexA = 4;
inline constexpr int kBossRocketIndexB = 6;
inline constexpr int kBossRocketLowHealthIndex = 9;

// AllMyPeople ally properties.
inline constexpr float kAllySpeed = 90.0f;
inline constexpr float kAllyHealth = 2'048.0f;
inline constexpr float kAllyDamage = 512.0f;
inline constexpr float kAllyBulletSpeed = 1'024.0f;
inline constexpr float kAllyFireInterval = 0.8f;
inline constexpr float kAllySpawnBelowScreen = 32.0f;
inline constexpr float kAllyHorizontalJitter = 16.0f;
inline constexpr float kAllyBulletSpawnOffset = 18.0f;
inline constexpr Vec2 kAllyCollisionSize = kEnemyCollisionSizes[0];

} // namespace thunder::config
