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

#include "GameInternals.hpp"
#include "thunder/game/Config.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <utility>

namespace thunder {
namespace {

void steer(Bullet &bullet, Vec2 target, float deltaTime) {
    const Vec2 desired = normalized(target - bullet.position, bullet.direction);
    const float currentAngle = angleOf(bullet.direction);
    const float targetAngle = angleOf(desired);
    const float difference = normalizeAngle(targetAngle - currentAngle);
    const float maxTurn = bullet.tracking * std::numbers::pi_v<float> * deltaTime;
    const float turn = std::clamp(difference, -maxTurn, maxTurn);
    bullet.direction = fromAngle(currentAngle + turn);
}

bool enemyOffscreen(const Enemy &enemy) {
    if (enemy.type == EnemyType::Sine) {
        // Preserve the original Enemy2 behaviour: only vertical bounds are tested.
        return enemy.position.y < 0.0f ||
               enemy.position.y > static_cast<float>(config::kScreenHeight);
    }
    return enemy.position.x < 0.0f ||
           enemy.position.x >= static_cast<float>(config::kScreenWidth) ||
           enemy.position.y < 0.0f || enemy.position.y > static_cast<float>(config::kScreenHeight);
}

} // namespace

GameWorld::GameWorld(std::uint64_t seed) : random_(seed) {
    player_.position = config::kPlayerStartPosition;
    player_.health = config::kPlayerMaxHealth * config::kPlayerInitialHealthRatio;
    player_.shotCooldown = config::kPlayerShotGap;
    player_.regenCooldown = config::kPlayerRecoverInterval;
    player_.animationTimer = config::kPlayerAnimationInterval;
    player_.textureIndex = 0;

    bullets_.reserve(2048);
    enemies_.reserve(128);
    allies_.reserve(320);
    explosions_.reserve(64);
}

GameWorld GameWorld::restore(const WorldState &state) {
    GameWorld world(0);
    world.random_.setState(state.random);
    world.elapsedTime_ = state.elapsed;
    world.kills_ = state.kills;
    world.alliesSummoned_ = state.alliesSummoned;
    world.enemySpawnTimer_ = state.enemySpawnTimer;
    world.giftSpawnTimer_ = state.giftSpawnTimer;

    world.player_ = Player{};
    world.player_.position = state.player.position;
    world.player_.health = state.player.health;
    world.player_.shotCooldown = state.player.shotCooldown;
    world.player_.regenCooldown = state.player.regenCooldown;
    world.player_.animationTimer = state.player.animationTimer;
    world.player_.textureIndex = state.player.textureIndex;
    world.player_.dying = state.player.dying;
    world.player_.deathFrame = state.player.deathFrame;
    world.player_.deathTimer = state.player.deathTimer;
    world.player_.dead = state.player.dead;
    world.player_.shotCount = state.player.shotCount;
    world.player_.gifts.clear();
    world.player_.gifts.reserve(state.player.gifts.size());
    for (const auto &saved : state.player.gifts) {
        Gift gift;
        gift.kind = saved.kind;
        gift.remaining = saved.remaining;
        gift.maxTime = saved.maxTime;
        gift.warning1Played = saved.warning1Played;
        gift.warning2Played = saved.warning2Played;
        gift.summonTimer = saved.summonTimer;
        gift.summonsRemaining = saved.summonsRemaining;
        world.player_.gifts.push_back(gift);
    }

    world.bullets_.clear();
    world.bullets_.reserve(state.bullets.size());
    for (const auto &saved : state.bullets) {
        world.bullets_.push_back(Bullet{saved.kind, saved.visual, saved.fromPlayer, saved.position,
                                        saved.direction, saved.speed, saved.damage,
                                        saved.damageRate, saved.tracking, saved.age});
    }

    world.enemies_.clear();
    world.enemies_.reserve(state.enemies.size());
    for (const auto &saved : state.enemies) {
        Enemy enemy;
        enemy.type = saved.type;
        enemy.position = saved.position;
        enemy.health = saved.health;
        enemy.maxHealth = saved.maxHealth;
        enemy.speed = saved.speed;
        enemy.bulletSpeed = saved.bulletSpeed;
        enemy.shotDamage = saved.shotDamage;
        enemy.killBonus = saved.killBonus;
        enemy.shotCooldown = saved.shotCooldown;
        enemy.shotGap = saved.shotGap;
        enemy.shotIndex = saved.shotIndex;
        enemy.motionTime = saved.motionTime;
        enemy.motionCenterX = saved.motionCenterX;
        enemy.motionAmplitude = saved.motionAmplitude;
        enemy.motionFrequency = saved.motionFrequency;
        enemy.hitFlash = saved.hitFlash;
        enemy.dying = saved.dying;
        enemy.deathFrame = saved.deathFrame;
        enemy.deathTimer = saved.deathTimer;
        enemy.rewarded = saved.rewarded;
        enemy.recoverRate = saved.recoverRate;
        world.enemies_.push_back(enemy);
    }

    world.allies_.clear();
    world.allies_.reserve(state.allies.size());
    for (const auto &saved : state.allies) {
        world.allies_.push_back(Ally{saved.position, saved.health, saved.fireCooldown});
    }

    world.explosions_.clear();
    world.sounds_.clear();
    return world;
}

WorldState GameWorld::snapshot() const {
    WorldState state;
    state.random = random_.state();
    state.elapsed = elapsedTime_;
    state.kills = kills_;
    state.alliesSummoned = alliesSummoned_;
    state.enemySpawnTimer = enemySpawnTimer_;
    state.giftSpawnTimer = giftSpawnTimer_;

    state.player.position = player_.position;
    state.player.health = player_.health;
    state.player.shotCooldown = player_.shotCooldown;
    state.player.regenCooldown = player_.regenCooldown;
    state.player.animationTimer = player_.animationTimer;
    state.player.textureIndex = player_.textureIndex;
    state.player.dying = player_.dying;
    state.player.deathFrame = player_.deathFrame;
    state.player.deathTimer = player_.deathTimer;
    state.player.dead = player_.dead;
    state.player.shotCount = player_.shotCount;
    state.player.gifts.reserve(player_.gifts.size());
    for (const auto &gift : player_.gifts) {
        state.player.gifts.push_back(GiftState{gift.kind, gift.remaining, gift.maxTime,
                                               gift.warning1Played, gift.warning2Played,
                                               gift.summonTimer, gift.summonsRemaining});
    }

    state.enemies.reserve(enemies_.size());
    for (const auto &enemy : enemies_) {
        state.enemies.push_back(EnemyState{
            enemy.type,          enemy.position,        enemy.health,          enemy.maxHealth,
            enemy.speed,         enemy.bulletSpeed,     enemy.shotDamage,      enemy.killBonus,
            enemy.shotCooldown,  enemy.shotGap,         enemy.shotIndex,       enemy.motionTime,
            enemy.motionCenterX, enemy.motionAmplitude, enemy.motionFrequency, enemy.hitFlash,
            enemy.dying,         enemy.deathFrame,      enemy.deathTimer,      enemy.rewarded,
            enemy.recoverRate});
    }

    state.bullets.reserve(bullets_.size());
    for (const auto &bullet : bullets_) {
        state.bullets.push_back(BulletState{
            bullet.kind, bullet.visual, bullet.fromPlayer, bullet.position, bullet.direction,
            bullet.speed, bullet.damage, bullet.damageRate, bullet.tracking, bullet.age});
    }

    state.allies.reserve(allies_.size());
    for (const auto &ally : allies_)
        state.allies.push_back(AllyState{ally.position, ally.health, ally.fireCooldown});

    return state;
}

void GameWorld::update(float fixedDeltaTime, const PlayerInput &input) {
    if (player_.dead)
        return;

    elapsedTime_ += fixedDeltaTime;

    updatePlayer(fixedDeltaTime, input);
    if (!player_.dying && !player_.dead)
        updatePlayerShooting(fixedDeltaTime);
    updateGifts(fixedDeltaTime);
    updateAllies(fixedDeltaTime);
    updateBullets(fixedDeltaTime);
    updateEnemies(fixedDeltaTime);
    resolveCollisions();
    updateExplosions(fixedDeltaTime);
    updateSpawning(fixedDeltaTime);

    if (player_.health <= 0.0f && !player_.dying && !player_.dead)
        startPlayerDeath();
}

void GameWorld::updatePlayer(float deltaTime, const PlayerInput &input) {
    if (player_.dead)
        return;

    if (player_.dying) {
        player_.deathTimer += deltaTime;
        while (player_.deathTimer >= config::kPlayerDeathFrameInterval) {
            player_.deathTimer -= config::kPlayerDeathFrameInterval;
            if (player_.deathFrame < config::kPlayerDeathFrameCount) {
                ++player_.deathFrame;
            } else {
                player_.dead = true;
                player_.dying = false;
                emitSound(SoundId::PlayerDown);
                return;
            }
        }
        return;
    }

    const float axisX = static_cast<float>(input.right) - static_cast<float>(input.left);
    const float axisY = static_cast<float>(input.down) - static_cast<float>(input.up);
    player_.position.x += axisX * config::kPlayerSpeed * deltaTime;
    player_.position.y += axisY * config::kPlayerSpeed * deltaTime;

    const Vec2 half = config::kPlayerCollisionSize * 0.5f;
    player_.position.x =
        clamp(player_.position.x, half.x, static_cast<float>(config::kScreenWidth) - half.x);
    player_.position.y =
        clamp(player_.position.y, half.y, static_cast<float>(config::kScreenHeight) - half.y);

    player_.animationTimer += deltaTime;
    while (player_.animationTimer >= config::kPlayerAnimationInterval) {
        player_.animationTimer -= config::kPlayerAnimationInterval;
        player_.textureIndex = (player_.textureIndex + 1) % 2;
    }

    player_.regenCooldown -= deltaTime;
    while (player_.regenCooldown <= 0.0f) {
        player_.health =
            std::min(config::kPlayerMaxHealth, player_.health + config::kPlayerRecoverHealth);
        player_.regenCooldown += config::kPlayerRecoverInterval;
    }

    if (player_.health <= 0.0f)
        startPlayerDeath();
}

void GameWorld::startPlayerDeath() {
    if (player_.dying || player_.dead)
        return;
    player_.health = 0.0f;
    player_.dying = true;
    player_.deathFrame = 1;
    player_.deathTimer = 0.0f;
}

void GameWorld::updatePlayerShooting(float deltaTime) {
    player_.shotCooldown -= deltaTime;
    if (player_.shotCooldown > 0.0f)
        return;

    const float multiplier = attackSpeedMultiplier();
    const bool rapidFire = multiplier > 1.0f;
    const Vec2 center = player_.position;

    addBullet(detail::makeCannon(
        center + Vec2{-config::kPlayerHorizontalShotOffset, config::kPlayerVerticalShotOffset},
        Vec2{0.0f, -1.0f}, BulletVisual::PlayerCannon, true, 1'024.0f, config::kPlayerDamage));
    addBullet(detail::makeCannon(
        center + Vec2{config::kPlayerHorizontalShotOffset, config::kPlayerVerticalShotOffset},
        Vec2{0.0f, -1.0f}, BulletVisual::PlayerCannon, true, 1'024.0f, config::kPlayerDamage));

    if (rapidFire) {
        if (player_.shotCount % 2U == 0U)
            emitSound(SoundId::Bullet3);
        for (int i = 0; i < config::kPlayerSuperBulletCount; ++i) {
            const Vec2 direction =
                normalized(Vec2{random_.uniform(-config::kPlayerSuperBulletSpread,
                                                config::kPlayerSuperBulletSpread),
                                -1.0f},
                           {0.0f, -1.0f});
            addBullet(
                detail::makeCannon(center + Vec2{0.0f, 2.0f * config::kPlayerVerticalShotOffset},
                                   direction, BulletVisual::PlayerSuper, true,
                                   config::kPlayerSuperBulletSpeed, config::kPlayerDamage));
        }
    }

    ++player_.shotCount;
    player_.shotCooldown = config::kPlayerShotGap / multiplier;
}

void GameWorld::updateBullets(float deltaTime) {
    for (auto &bullet : bullets_) {
        bullet.age += deltaTime;

        if (bullet.kind == BulletKind::Missile) {
            if (!bullet.fromPlayer) {
                if (bullet.tracking > 0.0f) {
                    bullet.speed += config::kMissileAcceleration * deltaTime;
                    bullet.tracking += config::kMissileTrackingAcceleration * deltaTime;
                    steer(bullet, homingTarget(), deltaTime);
                    if (bullet.age >= config::kMissileTrackingDuration)
                        bullet.tracking = 0.0f;
                } else {
                    bullet.speed += config::kMissileFallbackAcceleration * deltaTime;
                }
            }
        } else if (bullet.kind == BulletKind::Rocket) {
            if (!bullet.fromPlayer && bullet.age < config::kRocketTrackingDuration)
                bullet.tracking = config::kRocketTracking;
            else
                bullet.tracking = 0.0f;

            if (bullet.damageRate != config::kRocketDamageRate && bullet.age >= 0.04f)
                bullet.damageRate = config::kRocketDamageRate;

            if (bullet.tracking > 0.0f)
                steer(bullet, homingTarget(), deltaTime);
            bullet.speed += config::kRocketAcceleration * deltaTime;
        }

        bullet.position += bullet.direction * bullet.speed * deltaTime;
    }

    std::erase_if(bullets_, [](const Bullet &bullet) {
        return bullet.position.x < 0.0f ||
               bullet.position.x > static_cast<float>(config::kScreenWidth) ||
               bullet.position.y < 0.0f ||
               bullet.position.y > static_cast<float>(config::kScreenHeight);
    });
}

void GameWorld::updateEnemies(float deltaTime) {
    std::vector<Enemy> survivors;
    survivors.reserve(enemies_.size());

    for (auto &enemy : enemies_) {
        enemy.hitFlash = std::max(0.0f, enemy.hitFlash - deltaTime);

        if (enemy.type == EnemyType::Scout) {
            enemy.position.y += enemy.speed * deltaTime;
        } else if (enemy.type == EnemyType::Sine) {
            enemy.motionTime += deltaTime;
            enemy.position.y += enemy.speed * deltaTime;
            enemy.position.x =
                enemy.motionCenterX +
                std::sin(enemy.motionTime * enemy.motionFrequency) * enemy.motionAmplitude;
        } else {
            enemy.motionTime += deltaTime;
            if (enemy.position.y > static_cast<float>(config::kScreenHeight) / 4.0f)
                enemy.position.y += enemy.speed * deltaTime;
            enemy.position.x =
                enemy.motionCenterX +
                std::sin(enemy.motionTime * enemy.motionFrequency) * enemy.motionAmplitude;
        }

        if (enemyOffscreen(enemy))
            continue;

        if (enemy.dying) {
            enemy.deathTimer += deltaTime;
            bool remove = false;
            while (enemy.deathTimer >= 0.16f) {
                enemy.deathTimer -= 0.16f;
                if (enemy.deathFrame < enemyDeathFrameCount(enemy.type)) {
                    ++enemy.deathFrame;
                } else {
                    remove = true;
                    break;
                }
            }
            if (!remove)
                survivors.push_back(std::move(enemy));
            continue;
        }

        if (enemy.type == EnemyType::Boss) {
            enemy.health = std::min(enemy.maxHealth * config::kEnemy3RecoverCapMultiplier,
                                    enemy.health + enemy.recoverRate * deltaTime);
        }

        enemy.shotCooldown -= deltaTime;
        if (enemy.shotCooldown <= 0.0f) {
            const Aabb bounds = detail::enemyBounds(enemy);
            const float centerX = bounds.center().x;
            const float bottomY =
                enemy.position.y + bounds.size.y + config::kEnemyBulletSpawnOffset;

            if (enemy.type != EnemyType::Boss) {
                addBullet(detail::makeCannon(Vec2{centerX, bottomY}, Vec2{0.0f, 1.0f},
                                             BulletVisual::EnemyCannon, false, enemy.bulletSpeed,
                                             enemy.shotDamage));
            } else {
                for (int i = 0; i < 6; ++i) {
                    const Vec2 direction =
                        normalized(Vec2{random_.uniform(-0.32f, 0.32f), 1.0f}, {0.0f, 1.0f});
                    addBullet(detail::makeCannon(Vec2{centerX, bottomY}, direction,
                                                 BulletVisual::BossBullet, false, enemy.bulletSpeed,
                                                 enemy.shotDamage));
                }

                enemy.shotIndex = (enemy.shotIndex + 1) % config::kBossPatternCycle;
                if (enemy.shotIndex == config::kBossMissileIndex ||
                    enemy.health < config::kBossMissileHealthThreshold) {
                    const int missileCount =
                        enemy.health < enemy.maxHealth * config::kBossMissileLowHealthRatio ? 4 : 2;
                    for (int i = 0; i < missileCount; ++i) {
                        addBullet(detail::makeMissile(
                            Vec2{centerX - 50.0f - static_cast<float>(i) * 20.0f,
                                 bottomY - static_cast<float>(i) * 36.0f},
                            Vec2{0.0f, 1.0f}, false,
                            enemy.bulletSpeed * (0.08f + static_cast<float>(i) * 0.01f),
                            enemy.shotDamage * 4.2f, 0.4f + static_cast<float>(i) * 0.5f));
                        addBullet(detail::makeMissile(
                            Vec2{centerX + 50.0f + static_cast<float>(i) * 20.0f,
                                 bottomY - static_cast<float>(i) * 36.0f},
                            Vec2{0.0f, 1.0f}, false,
                            enemy.bulletSpeed * (0.02f + static_cast<float>(i) * 0.03f),
                            enemy.shotDamage * 4.2f, 0.4f + static_cast<float>(i) * 0.5f));
                    }
                    emitSound(SoundId::Missile);
                } else if (enemy.shotIndex == config::kBossRocketIndexA ||
                           enemy.shotIndex == config::kBossRocketIndexB ||
                           (enemy.health < enemy.maxHealth * config::kBossRocketLowHealthRatio &&
                            enemy.shotIndex == config::kBossRocketLowHealthIndex)) {
                    addBullet(detail::makeRocket(Vec2{centerX - 50.0f, bottomY}, Vec2{0.0f, 1.0f},
                                                 false, enemy.bulletSpeed * 0.01f,
                                                 enemy.shotDamage * 1.6f));
                    addBullet(detail::makeRocket(Vec2{centerX + 50.0f, bottomY}, Vec2{0.0f, 1.0f},
                                                 false, enemy.bulletSpeed * 0.14f,
                                                 enemy.shotDamage * 1.6f));
                    emitSound(SoundId::Rocket);
                } else {
                    emitSound(SoundId::Bullet);
                }
            }
            enemy.shotCooldown = enemy.shotGap;
        }

        survivors.push_back(std::move(enemy));
    }

    enemies_.swap(survivors);
}

void GameWorld::updateAllies(float deltaTime) {
    std::vector<Ally> survivors;
    survivors.reserve(allies_.size());

    for (auto &ally : allies_) {
        ally.position.y -= config::kAllySpeed * deltaTime;
        ally.fireCooldown -= deltaTime;
        if (ally.fireCooldown <= 0.0f) {
            const auto target = nearestEnemyCenter(ally.position);
            const Vec2 direction =
                target ? normalized(*target - ally.position, {0.0f, -1.0f}) : Vec2{0.0f, -1.0f};
            addBullet(detail::makeCannon(
                ally.position + Vec2{0.0f, -config::kAllyBulletSpawnOffset}, direction,
                BulletVisual::PlayerCannon, true, config::kAllyBulletSpeed, config::kAllyDamage));
            ally.fireCooldown = config::kAllyFireInterval;
        }

        const float bottom = ally.position.y + config::kAllyCollisionSize.y * 0.5f;
        if (bottom >= 0.0f)
            survivors.push_back(std::move(ally));
    }

    allies_.swap(survivors);
}

void GameWorld::updateGifts(float deltaTime) {
    std::vector<Gift> survivors;
    survivors.reserve(player_.gifts.size());

    for (auto &gift : player_.gifts) {
        gift.remaining -= deltaTime;

        if (!gift.warning1Played &&
            gift.remaining <= config::kGiftWarningTime - config::kGiftWarningDelay1) {
            emitSound(SoundId::GiftDisappear1);
            gift.warning1Played = true;
        }
        if (!gift.warning2Played &&
            gift.remaining <= config::kGiftWarningTime - config::kGiftWarningDelay2) {
            emitSound(SoundId::GiftDisappear1);
            gift.warning2Played = true;
        }

        if (giftIsAllMyPeople(gift.kind) && gift.summonsRemaining > 0) {
            gift.summonTimer -= deltaTime;
            while (gift.summonsRemaining > 0 && gift.summonTimer <= 0.0f) {
                spawnAllyWave();
                --gift.summonsRemaining;
                gift.summonTimer += 1.0f;
            }
        }

        if (gift.remaining > 0.0f) {
            survivors.push_back(std::move(gift));
        } else {
            emitSound(SoundId::GiftDisappear2);
        }
    }

    player_.gifts.swap(survivors);
}

void GameWorld::updateExplosions(float deltaTime) {
    for (auto &explosion : explosions_)
        explosion.timer -= deltaTime;
    std::erase_if(explosions_, [](const Explosion &explosion) { return explosion.timer <= 0.0f; });
}

void GameWorld::grantGift(GiftKind kind) {
    addGift(kind);
}

Aabb GameWorld::playerBounds() const {
    if (hasGift(GiftKind::CenturyShield))
        return Aabb::fromCenter(player_.position, config::kPlayerShieldCollisionSize);
    return Aabb::fromCenter(player_.position, config::kPlayerCollisionSize);
}

bool GameWorld::hasGift(GiftKind kind) const {
    return std::any_of(player_.gifts.begin(), player_.gifts.end(), [kind](const Gift &gift) {
        return gift.kind == kind && gift.remaining > 0.0f;
    });
}

float GameWorld::attackSpeedMultiplier() const {
    float multiplier = 1.0f;
    for (const auto &gift : player_.gifts)
        multiplier *= 1.0f + giftAttackSpeedIncrease(gift.kind);
    return multiplier;
}

float GameWorld::damageReductionMultiplier() const {
    float multiplier = 1.0f;
    for (const auto &gift : player_.gifts)
        multiplier *= 1.0f - giftDamageReduction(gift.kind);
    return multiplier;
}

int GameWorld::countEnemies(EnemyType type) const {
    return static_cast<int>(
        std::count_if(enemies_.begin(), enemies_.end(),
                      [type](const Enemy &enemy) { return enemy.type == type; }));
}

const Enemy *GameWorld::activeBoss() const {
    for (const auto &enemy : enemies_) {
        if (enemy.type == EnemyType::Boss && !enemy.dying && enemy.health > 0.0f)
            return &enemy;
    }
    return nullptr;
}

bool GameWorld::hasActiveBoss() const {
    return activeBoss() != nullptr;
}

bool GameWorld::allMyPeopleActive() const {
    return std::any_of(player_.gifts.begin(), player_.gifts.end(), [](const Gift &gift) {
        return gift.kind == GiftKind::AllMyPeople && gift.remaining > 0.0f;
    });
}

Vec2 GameWorld::homingTarget() const {
    return player_.position;
}

std::optional<Vec2> GameWorld::nearestEnemyCenter(Vec2 from) const {
    std::optional<Vec2> best;
    float bestDistance = std::numeric_limits<float>::max();
    for (const auto &enemy : enemies_) {
        if (enemy.dying)
            continue;
        const Vec2 center = detail::enemyCenter(enemy);
        const float distance = lengthSquared(center - from);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = center;
        }
    }
    return best;
}

void GameWorld::damageEnemy(Enemy &enemy, float damage) {
    if (enemy.dying)
        return;
    enemy.health -= damage;
    enemy.hitFlash = 0.08f;
    if (enemy.health > 0.0f)
        return;

    enemy.health = 0.0f;
    enemy.dying = true;
    enemy.deathFrame = 1;
    enemy.deathTimer = 0.0f;
    switch (enemy.type) {
        case EnemyType::Scout: emitSound(SoundId::Enemy1Down); break;
        case EnemyType::Sine: emitSound(SoundId::Enemy2Down); break;
        case EnemyType::Boss: emitSound(SoundId::Enemy3Down); break;
    }

    if (!enemy.rewarded) {
        enemy.rewarded = true;
        ++kills_;
        player_.health += enemy.killBonus;
    }
}

void GameWorld::addExplosion(ExplosionKind kind) {
    explosions_.push_back(Explosion{kind, config::kExplosionDuration});
}

std::vector<SoundId> GameWorld::takeSounds() {
    std::vector<SoundId> sounds = std::move(sounds_);
    sounds_.clear();
    return sounds;
}

} // namespace thunder
