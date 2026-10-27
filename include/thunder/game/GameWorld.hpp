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
#include "thunder/game/GameState.hpp"
#include "thunder/game/Types.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace thunder {

/// Pure gameplay simulation. It has no knowledge of windows, textures, fonts,
/// audio or files and can be driven at a fixed timestep from tests.
class GameWorld {
public:
    explicit GameWorld(std::uint64_t seed = makeSeed());
    static GameWorld restore(const WorldState &state);

    void update(float fixedDeltaTime, const PlayerInput &input);
    void grantGift(GiftKind kind);

    [[nodiscard]] WorldState snapshot() const;

    [[nodiscard]] const Player &player() const {
        return player_;
    }

    [[nodiscard]] std::span<const Gift> gifts() const {
        return player_.gifts;
    }

    [[nodiscard]] std::span<const Bullet> bullets() const {
        return bullets_;
    }

    [[nodiscard]] std::span<const Enemy> enemies() const {
        return enemies_;
    }

    [[nodiscard]] std::span<const Ally> allies() const {
        return allies_;
    }

    [[nodiscard]] std::span<const Explosion> explosions() const {
        return explosions_;
    }

    [[nodiscard]] const Enemy *activeBoss() const;
    [[nodiscard]] bool allMyPeopleActive() const;

    [[nodiscard]] bool playerDefeated() const {
        return player_.dead;
    }

    [[nodiscard]] float elapsedTime() const {
        return elapsedTime_;
    }

    [[nodiscard]] std::uint64_t kills() const {
        return kills_;
    }

    [[nodiscard]] std::uint64_t alliesSummoned() const {
        return alliesSummoned_;
    }

    [[nodiscard]] std::vector<SoundId> takeSounds();

private:
    void updatePlayer(float deltaTime, const PlayerInput &input);
    void updatePlayerShooting(float deltaTime);
    void updateBullets(float deltaTime);
    void updateEnemies(float deltaTime);
    void updateAllies(float deltaTime);
    void updateGifts(float deltaTime);
    void updateExplosions(float deltaTime);
    void updateSpawning(float deltaTime);
    void resolveCollisions();

    void trySpawnEnemy();
    void trySpawnGift();
    void spawnBossWave();
    void spawnEnemy(EnemyType type, Vec2 position);
    void spawnAllyWave();
    void addGift(GiftKind kind);
    void damageEnemy(Enemy &enemy, float damage);
    void startPlayerDeath();

    [[nodiscard]] Aabb playerBounds() const;
    [[nodiscard]] bool hasGift(GiftKind kind) const;
    [[nodiscard]] float attackSpeedMultiplier() const;
    [[nodiscard]] float damageReductionMultiplier() const;
    [[nodiscard]] int countEnemies(EnemyType type) const;
    [[nodiscard]] bool hasActiveBoss() const;
    [[nodiscard]] Vec2 homingTarget() const;
    [[nodiscard]] std::optional<Vec2> nearestEnemyCenter(Vec2 from) const;

    void emitSound(SoundId sound) {
        sounds_.push_back(sound);
    }

    void addExplosion(ExplosionKind kind);

    void addBullet(const Bullet &bullet) {
        bullets_.push_back(bullet);
    }

    Random random_;
    Player player_;
    std::vector<Bullet> bullets_;
    std::vector<Enemy> enemies_;
    std::vector<Ally> allies_;
    std::vector<Explosion> explosions_;
    std::vector<SoundId> sounds_;

    float elapsedTime_{};
    std::uint64_t kills_{};
    std::uint64_t alliesSummoned_{};
    float enemySpawnTimer_{};
    float giftSpawnTimer_{};
};

} // namespace thunder
