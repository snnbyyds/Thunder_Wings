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

#include "thunder/game/Config.hpp"
#include "thunder/game/Types.hpp"

namespace thunder::detail {

inline Aabb enemyBounds(const Enemy &enemy) {
    return {enemy.position,
            config::kEnemyCollisionSizes[static_cast<std::size_t>(enemy.type) - 1U]};
}

inline Vec2 enemyCenter(const Enemy &enemy) {
    return enemyBounds(enemy).center();
}

inline Aabb bulletBounds(const Bullet &bullet) {
    const auto index = static_cast<std::size_t>(bullet.visual);
    return Aabb::fromCenter(bullet.position, config::kBulletCollisionSizes[index]);
}

inline Bullet makeCannon(Vec2 position, Vec2 direction, BulletVisual visual, bool fromPlayer,
                         float speed, float damage) {
    return Bullet{BulletKind::Cannon,
                  visual,
                  fromPlayer,
                  position,
                  normalized(direction, {0.0f, 1.0f}),
                  speed,
                  damage,
                  0.0f,
                  0.0f,
                  0.0f};
}

inline Bullet makeMissile(Vec2 position, Vec2 direction, bool fromPlayer, float speed, float damage,
                          float tracking) {
    return Bullet{BulletKind::Missile,
                  BulletVisual::Missile,
                  fromPlayer,
                  position,
                  normalized(direction, {0.0f, 1.0f}),
                  speed,
                  damage,
                  0.0f,
                  fromPlayer ? 0.0f : tracking,
                  0.0f};
}

inline Bullet makeRocket(Vec2 position, Vec2 direction, bool fromPlayer, float speed,
                         float damage) {
    return Bullet{BulletKind::Rocket,
                  BulletVisual::Rocket,
                  fromPlayer,
                  position,
                  normalized(direction, {0.0f, 1.0f}),
                  speed,
                  damage,
                  config::kRocketDamageRateInitial,
                  0.0f,
                  0.0f};
}

inline bool offscreen(Vec2 position, float margin = 0.0f) {
    return position.x < -margin || position.x > config::kScreenWidth + margin ||
           position.y < -margin || position.y > config::kScreenHeight + margin;
}

inline int levelIndex(EnemyType type) {
    return static_cast<int>(type) - 1;
}

} // namespace thunder::detail
