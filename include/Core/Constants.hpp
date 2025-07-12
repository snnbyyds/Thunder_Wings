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
#include <cstddef>

namespace Constants {
// clang-format off

// Screen Properties
constexpr int SCREEN_WIDTH  = 1440;
constexpr int SCREEN_HEIGHT = 900;

// Player Properties
constexpr size_t PLAYER_BULLET_ID     = 1ul;
constexpr float PLAYER_SPEED          = 512.0f;
constexpr float PLAYER_MAX_HEALTH     = 96000.0f;
constexpr float PLAYER_DAMAGE         = 512.0f;
constexpr float PLAYER_SHOT_GAP       = 0.016f;
constexpr float PLAYER_RECOVER_HEALTH = 8.0f;

// Enemy Properties
constexpr size_t ENEMY_BULLET_ID         = 0ul;
constexpr float ENEMY_SPAWN_INTERVAL_MIN = 0.32f;
constexpr float ENEMY_SPAWN_INTERVAL_MAX = 1.28f;
constexpr float ENEMY1_SPAWN_PROB        = 0.65f;
constexpr float ENEMY2_SPAWN_PROB        = 1.0f - ENEMY1_SPAWN_PROB;

// clang-format on
} // namespace Constants