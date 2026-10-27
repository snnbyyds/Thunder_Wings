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

#include "thunder/game/Config.hpp"
#include "thunder/game/GameWorld.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

namespace {

using thunder::BulletState;
using thunder::EnemyState;
using thunder::EnemyType;
using thunder::GameWorld;
using thunder::GiftKind;
using thunder::PlayerInput;
using thunder::WorldState;

void advance(GameWorld &world, float seconds, PlayerInput input = {}) {
    const int steps =
        std::max(1, static_cast<int>(std::lround(seconds / thunder::config::kFixedTimeStep)));
    for (int i = 0; i < steps; ++i)
        world.update(thunder::config::kFixedTimeStep, input);
}

WorldState baseState() {
    WorldState state;
    // Keep deterministic tests free of random spawns.
    state.enemySpawnTimer = -1000.0f;
    state.giftSpawnTimer = -1000.0f;
    state.player.position = {720.0f, 500.0f};
    state.player.health = thunder::config::kPlayerMaxHealth;
    state.player.shotCooldown = 999.0f;
    state.player.regenCooldown = 999.0f;
    state.player.animationTimer = 0.0f;
    state.player.textureIndex = 0;
    state.player.dying = false;
    state.player.deathFrame = 0;
    state.player.deathTimer = 0.0f;
    state.player.dead = false;
    state.player.shotCount = 0;
    return state;
}

EnemyState makeEnemy(thunder::Vec2 position, float health = 2048.0f) {
    EnemyState enemy;
    enemy.type = EnemyType::Scout;
    enemy.position = position;
    enemy.health = health;
    enemy.maxHealth = health;
    enemy.speed = 0.0f;
    enemy.bulletSpeed = 0.0f;
    enemy.shotDamage = 0.0f;
    enemy.killBonus = 5000.0f;
    enemy.shotCooldown = 100.0f;
    enemy.shotGap = 100.0f;
    enemy.shotIndex = 0;
    enemy.motionTime = 0.0f;
    enemy.motionCenterX = position.x;
    enemy.motionAmplitude = 0.0f;
    enemy.motionFrequency = 0.0f;
    enemy.hitFlash = 0.0f;
    enemy.dying = false;
    enemy.deathFrame = 0;
    enemy.deathTimer = 0.0f;
    enemy.rewarded = false;
    enemy.recoverRate = 0.0f;
    return enemy;
}

BulletState enemyBullet(thunder::Vec2 position, float damage = 512.0f) {
    BulletState bullet;
    bullet.kind = thunder::BulletKind::Cannon;
    bullet.visual = thunder::BulletVisual::EnemyCannon;
    bullet.fromPlayer = false;
    bullet.position = position;
    bullet.direction = {0.0f, 0.0f};
    bullet.speed = 0.0f;
    bullet.damage = damage;
    bullet.damageRate = 0.0f;
    bullet.tracking = 0.0f;
    bullet.age = 0.0f;
    return bullet;
}

BulletState playerBullet(thunder::Vec2 position, float damage = 6144.0f) {
    BulletState bullet;
    bullet.kind = thunder::BulletKind::Cannon;
    bullet.visual = thunder::BulletVisual::PlayerCannon;
    bullet.fromPlayer = true;
    bullet.position = position;
    bullet.direction = {0.0f, 0.0f};
    bullet.speed = 0.0f;
    bullet.damage = damage;
    bullet.damageRate = 0.0f;
    bullet.tracking = 0.0f;
    bullet.age = 0.0f;
    return bullet;
}

int bulletsFromPlayer(const GameWorld &world) {
    return static_cast<int>(
        std::count_if(world.bullets().begin(), world.bullets().end(),
                      [](const thunder::Bullet &bullet) { return bullet.fromPlayer; }));
}

} // namespace

TEST(GameWorldTest, PlayerMovementClampsToScreen) {
    WorldState state = baseState();
    state.player.position = {1400.0f, 500.0f};
    GameWorld world = GameWorld::restore(state);
    advance(world, 0.2f, PlayerInput{false, true, false, false});
    const float halfWidth = thunder::config::kPlayerCollisionSize.x * 0.5f;
    EXPECT_LE(world.player().position.x,
              static_cast<float>(thunder::config::kScreenWidth) - halfWidth + 0.01f);

    state.player.position = {10.0f, 500.0f};
    GameWorld leftWorld = GameWorld::restore(state);
    advance(leftWorld, 0.2f, PlayerInput{true, false, false, false});
    EXPECT_GE(leftWorld.player().position.x, halfWidth - 0.01f);
}

TEST(GameWorldTest, PlayerRecoversHealthOnInterval) {
    WorldState state = baseState();
    state.player.health = 1000.0f;
    state.player.regenCooldown = thunder::config::kPlayerRecoverInterval;
    GameWorld world = GameWorld::restore(state);
    advance(world, 0.65f);
    EXPECT_FLOAT_EQ(world.player().health, 1000.0f + thunder::config::kPlayerRecoverHealth);
}

TEST(GameWorldTest, PlayerShootsAndFullFirePowerAddsSpread) {
    GameWorld world(7);
    advance(world, 0.15f);
    EXPECT_EQ(bulletsFromPlayer(world), 2);

    world.grantGift(GiftKind::FullFirePower);
    advance(world, 0.15f);
    EXPECT_GE(bulletsFromPlayer(world), 10);
}

TEST(GameWorldTest, CenturyShieldReducesEnemyBulletDamage) {
    WorldState state = baseState();
    state.bullets.push_back(enemyBullet(state.player.position));
    GameWorld world = GameWorld::restore(state);
    world.grantGift(GiftKind::CenturyShield);
    const float before = world.player().health;
    advance(world, thunder::config::kFixedTimeStep);
    EXPECT_NEAR(world.player().health, before - 512.0f * 0.1f, 0.1f);
}

TEST(GameWorldTest, KillingEnemyRewardsPlayerAndCountsKill) {
    WorldState state = baseState();
    state.enemies.push_back(makeEnemy({600.0f, 300.0f}));
    state.bullets.push_back(playerBullet({628.5f, 321.5f}));
    GameWorld world = GameWorld::restore(state);
    const float before = world.player().health;
    advance(world, thunder::config::kFixedTimeStep);
    ASSERT_EQ(world.enemies().size(), 1U);
    EXPECT_TRUE(world.enemies()[0].dying);
    EXPECT_EQ(world.kills(), 1U);
    EXPECT_FLOAT_EQ(world.player().health, before + 5000.0f);
}

TEST(GameWorldTest, AllMyPeopleSpawns256AlliesPerSecondForSixteenSeconds) {
    GameWorld world(12345);
    world.grantGift(GiftKind::AllMyPeople);
    advance(world, thunder::config::kFixedTimeStep);
    EXPECT_EQ(world.alliesSummoned(), 256U);
    ASSERT_EQ(world.gifts().size(), 1U);
    EXPECT_EQ(world.gifts()[0].summonsRemaining, 15);

    const float firstY = world.allies()[0].position.y;
    advance(world, 1.0f);
    EXPECT_LT(world.allies()[0].position.y, firstY);
    EXPECT_EQ(world.alliesSummoned(), 512U);

    advance(world, 15.1f);
    EXPECT_EQ(world.alliesSummoned(), 4096U);
    EXPECT_TRUE(world.gifts().empty());
    EXPECT_FALSE(world.allMyPeopleActive());
}

TEST(GameWorldTest, AlliesFightNearestEnemyWithoutPlayerShooting) {
    WorldState state = baseState();
    state.enemies.push_back(makeEnemy({700.0f, 250.0f}, 100000.0f));
    GameWorld world = GameWorld::restore(state);
    world.grantGift(GiftKind::AllMyPeople);
    advance(world, 2.0f);
    ASSERT_EQ(world.enemies().size(), 1U);
    EXPECT_LT(world.enemies()[0].health, 100000.0f);
}

TEST(GameWorldTest, PlayerDiesAfterHealthIsDepleted) {
    WorldState state = baseState();
    state.player.health = 1.0f;
    state.bullets.push_back(enemyBullet(state.player.position, 10.0f));
    GameWorld world = GameWorld::restore(state);
    advance(world, 0.1f);
    EXPECT_TRUE(world.player().dying || world.player().dead);
    advance(world, 2.0f);
    EXPECT_TRUE(world.playerDefeated());
}
