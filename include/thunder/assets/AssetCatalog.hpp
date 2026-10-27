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

#include "thunder/game/Types.hpp"

#include <cstddef>
#include <string_view>

namespace thunder {

enum class TextureId : std::size_t {
    Background,
    Player1,
    Player2,
    PlayerDeath1,
    PlayerDeath2,
    PlayerDeath3,
    PlayerDeath4,
    PlayerShield,
    Enemy1,
    Enemy1Hit,
    Enemy1Down1,
    Enemy1Down2,
    Enemy1Down3,
    Enemy1Down4,
    Enemy2,
    Enemy2Hit,
    Enemy2Down1,
    Enemy2Down2,
    Enemy2Down3,
    Enemy2Down4,
    Enemy3,
    Enemy3Hit,
    Enemy3Down1,
    Enemy3Down2,
    Enemy3Down3,
    Enemy3Down4,
    Enemy3Down5,
    Enemy3Down6,
    BulletEnemyCannon,
    BulletPlayerCannon,
    Missile,
    Rocket,
    BulletPlayerSuper,
    BulletBoss,
    ExplosionMissile,
    ExplosionRocket,
    GiftFullFirePower,
    GiftCenturyShield,
    GiftAllMyPeople,
    Count,
};

enum class FontId : std::size_t {
    Game,
    Page,
    Count,
};

[[nodiscard]] std::string_view texturePath(TextureId texture);
[[nodiscard]] bool textureSmooth(TextureId texture);
[[nodiscard]] std::string_view fontPath(FontId font);
[[nodiscard]] std::string_view soundPath(SoundId sound);
[[nodiscard]] std::string_view musicPath();

[[nodiscard]] TextureId enemyTexture(EnemyType type);
[[nodiscard]] TextureId enemyHitTexture(EnemyType type);
[[nodiscard]] TextureId enemyDeathTexture(EnemyType type, int frame);
[[nodiscard]] TextureId bulletTexture(BulletVisual visual);
[[nodiscard]] TextureId giftTexture(GiftKind kind);
[[nodiscard]] TextureId playerDeathTexture(int frame);

} // namespace thunder
