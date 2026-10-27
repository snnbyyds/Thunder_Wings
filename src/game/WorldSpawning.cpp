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

#include "thunder/game/GameWorld.hpp"

#include "thunder/game/Config.hpp"

#include <algorithm>
#include <array>

namespace thunder {
namespace {

Enemy makeEnemy(EnemyType type, Vec2 position, Random &random) {
    Enemy enemy;
    enemy.type = type;
    enemy.position = position;
    enemy.shotIndex = 0;
    enemy.motionTime = 0.0f;
    enemy.motionCenterX = position.x;
    enemy.hitFlash = 0.0f;
    enemy.dying = false;
    enemy.deathFrame = 0;
    enemy.deathTimer = 0.0f;
    enemy.rewarded = false;

    switch (type) {
        case EnemyType::Scout:
            enemy.maxHealth = config::kEnemy1Health;
            enemy.speed = random.uniform(config::kEnemy1SpeedMin, config::kEnemy1SpeedMax);
            enemy.bulletSpeed = enemy.speed * config::kEnemy1BulletSpeedFactor;
            enemy.shotGap = config::kEnemy1ShotGap;
            enemy.shotDamage = config::kEnemy1Damage;
            enemy.killBonus = config::kEnemy1KillBonus;
            enemy.motionAmplitude = 0.0f;
            enemy.motionFrequency = 0.0f;
            enemy.recoverRate = 0.0f;
            break;
        case EnemyType::Sine:
            enemy.maxHealth = config::kEnemy2Health;
            enemy.speed = random.uniform(config::kEnemy2SpeedMin, config::kEnemy2SpeedMax);
            enemy.bulletSpeed = enemy.speed * config::kEnemy2BulletSpeedFactor;
            enemy.shotGap = config::kEnemy2ShotGap;
            enemy.shotDamage = config::kEnemy2Damage;
            enemy.killBonus = config::kEnemy2KillBonus;
            enemy.motionAmplitude = random.uniform(128.0f, 256.0f);
            enemy.motionFrequency = random.uniform(0.5f, 1.5f);
            enemy.recoverRate = 0.0f;
            break;
        case EnemyType::Boss:
            enemy.maxHealth = random.uniform(config::kEnemy3HealthMin, config::kEnemy3HealthMax);
            enemy.speed = random.uniform(config::kEnemy3SpeedMin, config::kEnemy3SpeedMax);
            enemy.bulletSpeed =
                random.uniform(config::kEnemy3HomingSpeedMin, config::kEnemy3HomingSpeedMax);
            enemy.shotGap = random.uniform(config::kEnemy3ShotGapMin, config::kEnemy3ShotGapMax);
            enemy.shotDamage = config::kEnemy3Damage;
            enemy.killBonus = enemy.maxHealth;
            enemy.motionAmplitude = random.uniform(128.0f, 256.0f);
            enemy.motionFrequency = random.uniform(0.6f, 1.2f);
            enemy.recoverRate = config::kEnemy3RecoverRate;
            break;
    }

    enemy.health = enemy.maxHealth;
    enemy.shotCooldown = enemy.shotGap;
    return enemy;
}

} // namespace

void GameWorld::updateSpawning(float deltaTime) {
    enemySpawnTimer_ += deltaTime;
    const float enemyInterval =
        hasActiveBoss() ? config::kBossSpawnInterval : config::kEnemySpawnInterval;
    if (enemySpawnTimer_ >= enemyInterval) {
        trySpawnEnemy();
        enemySpawnTimer_ = 0.0f;
    }

    giftSpawnTimer_ += deltaTime;
    trySpawnGift();
}

void GameWorld::trySpawnEnemy() {
    const std::size_t choice = random_.weightedIndex(config::kEnemyLevelWeights);
    if (choice == 2U) {
        if (elapsedTime_ > config::kBossSpawnTime &&
            countEnemies(EnemyType::Boss) < config::kEnemy3MaxAlive)
            spawnBossWave();
        return;
    }

    const EnemyType type = choice == 0U ? EnemyType::Scout : EnemyType::Sine;
    const int alive = countEnemies(type);
    const int maximum =
        type == EnemyType::Scout ? config::kEnemy1MaxAlive : config::kEnemy2MaxAlive;
    if (alive >= maximum)
        return;

    const int x = random_.uniformInt(0, config::kScreenWidth - 1);
    spawnEnemy(type, Vec2{static_cast<float>(x), 0.0f});
}

void GameWorld::spawnEnemy(EnemyType type, Vec2 position) {
    enemies_.push_back(makeEnemy(type, position, random_));
}

void GameWorld::spawnBossWave() {
    for (int i = 0; i < config::kBossWaveScouts; ++i) {
        const int x = random_.uniformInt(0, config::kScreenWidth - 1);
        spawnEnemy(EnemyType::Scout, Vec2{static_cast<float>(x), 0.0f});
    }
    for (int i = 0; i < config::kBossWaveSine; ++i) {
        const int x = random_.uniformInt(0, config::kScreenWidth - 1);
        spawnEnemy(EnemyType::Sine, Vec2{static_cast<float>(x), 0.0f});
    }
    spawnEnemy(EnemyType::Boss, Vec2{static_cast<float>(config::kScreenWidth) / 2.0f, 0.0f});
}

void GameWorld::trySpawnGift() {
    if (player_.gifts.size() >= config::kMaxActiveGifts)
        return;
    if (giftSpawnTimer_ < config::kGiftSpawnInterval)
        return;
    if (!random_.chance(config::kGiftSpawnProbability))
        return;

    const std::size_t available = config::kMaxActiveGifts - player_.gifts.size();
    std::size_t count =
        random_.chance(config::kGiftTripleChance) ? (hasActiveBoss() ? 3U : 2U) : 1U;
    count = std::min(count, available);
    count = std::min(count, config::kGiftKinds.size());

    std::array<float, config::kGiftWeights.size()> weights = config::kGiftWeights;
    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t index = random_.weightedIndex(weights);
        addGift(config::kGiftKinds[index]);
        weights[index] = 0.0f;
    }
    giftSpawnTimer_ = 0.0f;
}

void GameWorld::addGift(GiftKind kind) {
    Gift gift;
    gift.kind = kind;
    if (kind == GiftKind::AllMyPeople) {
        gift.maxTime = config::kAllMyPeopleDuration;
        gift.summonsRemaining = config::kAllMyPeopleSummons;
        gift.summonTimer = 0.0f;
    } else {
        gift.maxTime = static_cast<float>(
            random_.uniformInt(config::kGiftDurationMin, config::kGiftDurationMax));
    }
    gift.remaining = gift.maxTime;
    gift.warning1Played = false;
    gift.warning2Played = false;
    player_.gifts.push_back(gift);

    switch (kind) {
        case GiftKind::FullFirePower: emitSound(SoundId::FullFirePower); break;
        case GiftKind::CenturyShield: emitSound(SoundId::CenturyShield); break;
        case GiftKind::AllMyPeople: emitSound(SoundId::AllMyPeople); break;
    }
}

void GameWorld::spawnAllyWave() {
    constexpr int count = config::kAllMyPeoplePerWave;
    const float columnWidth =
        static_cast<float>(config::kScreenWidth) / static_cast<float>(config::kAllMyPeopleColumns);
    for (int i = 0; i < count; ++i) {
        const int row = i / config::kAllMyPeopleColumns;
        const int column = i % config::kAllMyPeopleColumns;
        const float baseX = (static_cast<float>(column) + 0.5f) * columnWidth;
        const float x = clamp(
            baseX + random_.uniform(-config::kAllyHorizontalJitter, config::kAllyHorizontalJitter),
            0.0f, static_cast<float>(config::kScreenWidth) - 1.0f);
        const float y = static_cast<float>(config::kScreenHeight) + config::kAllySpawnBelowScreen +
                        static_cast<float>(row) * config::kAllyRowSpacing;
        Ally ally;
        ally.position = {x, y};
        ally.health = config::kAllyHealth;
        ally.fireCooldown = random_.uniform(0.05f, 0.9f);
        allies_.push_back(ally);
        ++alliesSummoned_;
    }
}

} // namespace thunder
