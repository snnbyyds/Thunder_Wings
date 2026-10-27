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
#include <utility>
#include <vector>

namespace thunder {

void GameWorld::resolveCollisions() {
    std::vector<char> bulletRemoved(bullets_.size(), 0);

    // Enemy bullets against the player.
    if (!player_.dying && !player_.dead) {
        const Aabb bounds = playerBounds();
        for (std::size_t i = 0; i < bullets_.size(); ++i) {
            const Bullet &bullet = bullets_[i];
            if (bulletRemoved[i] || bullet.fromPlayer)
                continue;
            if (!bounds.intersects(detail::bulletBounds(bullet)))
                continue;

            const float rawDamage = std::max(bullet.damage, bullet.damageRate * player_.health);
            player_.health -= rawDamage * damageReductionMultiplier();
            if (bullet.kind == BulletKind::Missile || bullet.kind == BulletKind::Rocket) {
                addExplosion(bullet.kind == BulletKind::Missile ? ExplosionKind::Missile
                                                                : ExplosionKind::Rocket);
                emitSound(SoundId::Explode);
            }
            bulletRemoved[i] = 1;
        }
    }

    // Enemy bullets against friendly units.
    std::vector<char> allyRemoved(allies_.size(), 0);
    for (std::size_t a = 0; a < allies_.size(); ++a) {
        if (allyRemoved[a])
            continue;
        Aabb bounds = Aabb::fromCenter(allies_[a].position, config::kAllyCollisionSize);
        for (std::size_t i = 0; i < bullets_.size(); ++i) {
            const Bullet &bullet = bullets_[i];
            if (bulletRemoved[i] || bullet.fromPlayer)
                continue;
            if (!bounds.intersects(detail::bulletBounds(bullet)))
                continue;

            const float rawDamage = std::max(bullet.damage, bullet.damageRate * allies_[a].health);
            allies_[a].health -= rawDamage;
            bulletRemoved[i] = 1;
            if (allies_[a].health <= 0.0f) {
                allyRemoved[a] = 1;
                emitSound(SoundId::Enemy1Down);
                break;
            }
        }
    }

    // Player and friendly bullets against enemies.
    for (auto &enemy : enemies_) {
        if (enemy.dying)
            continue;
        const Aabb bounds = detail::enemyBounds(enemy);
        for (std::size_t i = 0; i < bullets_.size(); ++i) {
            const Bullet &bullet = bullets_[i];
            if (bulletRemoved[i] || !bullet.fromPlayer)
                continue;
            if (!bounds.intersects(detail::bulletBounds(bullet)))
                continue;

            const float damage = std::max(bullet.damage, bullet.damageRate * enemy.health);
            bulletRemoved[i] = 1;
            if (bullet.kind == BulletKind::Missile || bullet.kind == BulletKind::Rocket)
                emitSound(SoundId::Explode);
            damageEnemy(enemy, damage);
            if (enemy.dying)
                break;
        }
    }

    if (player_.health <= 0.0f && !player_.dying && !player_.dead)
        startPlayerDeath();

    std::vector<Bullet> survivingBullets;
    survivingBullets.reserve(bullets_.size());
    for (std::size_t i = 0; i < bullets_.size(); ++i) {
        if (!bulletRemoved[i])
            survivingBullets.push_back(std::move(bullets_[i]));
    }
    bullets_.swap(survivingBullets);

    std::vector<Ally> survivingAllies;
    survivingAllies.reserve(allies_.size());
    for (std::size_t i = 0; i < allies_.size(); ++i) {
        if (!allyRemoved[i])
            survivingAllies.push_back(std::move(allies_[i]));
    }
    allies_.swap(survivingAllies);
}

} // namespace thunder
