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

// Common Properties
constexpr const char *BACKGROUND_FILE_NAME = "assets/background.png";
constexpr const char *BGM_FILE_NAME = "assets/background.wav";
constexpr const char *GAME_FONT = "assets/Morning Routine.otf";
constexpr const char *PARAGRAPH_FONT = "assets/NotoSans-MediumItalic.ttf";
constexpr float LOGO_DURATION = 3.0f;

// Screen Properties
constexpr int SCREEN_WIDTH  = 1440;
constexpr int SCREEN_HEIGHT = 900;

// Player Properties
constexpr size_t PLAYER_BULLET_ID     = 1ul;
constexpr float PLAYER_SPEED          = 512.0f;
constexpr float PLAYER_MAX_HEALTH     = 1024000.0f;
constexpr float PLAYER_DAMAGE         = 6144.0f;
constexpr float PLAYER_SHOT_GAP       = 0.144f;
constexpr float PLAYER_RECOVER_HEALTH = 128.0f;

// Enemy Properties
constexpr size_t ENEMY_BULLET_ID         = 0ul;
constexpr size_t ENEMY_MISSILE_ID        = 2ul;
constexpr size_t ENEMY_ROCKET_ID         = 3ul;
constexpr float ENEMY_SPAWN_INTERVAL_MIN = 0.32f;
constexpr float ENEMY_SPAWN_INTERVAL_MAX = 1.28f;
constexpr float ENEMY1_SPAWN_PROB        = 0.6f;
constexpr int ENEMY1_MAX_ALIVE           = 32;
constexpr float ENEMY2_SPAWN_PROB        = 0.38f;
constexpr int ENEMY2_MAX_ALIVE           = 32;
constexpr float ENEMY3_SPAWN_PROB        = 0.02f;
constexpr int ENEMY3_MAX_ALIVE           = 1;
constexpr size_t ENEMY_LEVEL_COUNT       = 3;

// Bullet Properties
constexpr float ROCKET_DAMAGE_RATE_INITIAL = 0.84f;
constexpr float ROCKET_DAMAGE_RATE         = 0.184f;

// Gift Properties
constexpr float GIFT_SPAWN_PROBABILITY = 0.48f;
constexpr float GIFT_SPAWN_INTERVAL    = 32.0f;
// clang-format on
} // namespace Constants