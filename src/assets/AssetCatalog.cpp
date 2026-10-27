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

#include "thunder/assets/AssetCatalog.hpp"

#include <algorithm>
#include <array>
#include <cstddef>

namespace thunder {
namespace {

constexpr std::size_t kTextureCount = static_cast<std::size_t>(TextureId::Count);
constexpr std::size_t kFontCount = static_cast<std::size_t>(FontId::Count);
constexpr std::size_t kSoundCount = static_cast<std::size_t>(SoundId::Count);

constexpr auto kTexturePaths = std::to_array<std::string_view>({
    "background.png",    "me1.png",           "me2.png",          "me_destroy_1.png",
    "me_destroy_2.png",  "me_destroy_3.png",  "me_destroy_4.png", "PlayerShield.png",
    "enemy1.png",        "enemy1_hit.png",    "enemy1_down1.png", "enemy1_down2.png",
    "enemy1_down3.png",  "enemy1_down4.png",  "enemy2.png",       "enemy2_hit.png",
    "enemy2_down1.png",  "enemy2_down2.png",  "enemy2_down3.png", "enemy2_down4.png",
    "enemy3.png",        "enemy3_hit.png",    "enemy3_down1.png", "enemy3_down2.png",
    "enemy3_down3.png",  "enemy3_down4.png",  "enemy3_down5.png", "enemy3_down6.png",
    "bullet1.png",       "bullet2.png",       "missle.png",       "rocket.png",
    "bullet3.png",       "bullet4.png",       "explode.png",      "explode2.png",
    "FullFirePower.png", "CenturyShield.png", "AllMyPeople.png",
});
static_assert(kTexturePaths.size() == kTextureCount);

constexpr auto kTextureSmooth = std::to_array<bool>({
    false, true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
});
static_assert(kTextureSmooth.size() == kTextureCount);

constexpr auto kFontPaths = std::to_array<std::string_view>({
    "Morning Routine.otf",
    "NotoSans-MediumItalic.ttf",
});
static_assert(kFontPaths.size() == kFontCount);

constexpr auto kSoundPaths = std::to_array<std::string_view>({
    "pause.wav",
    "bullet.wav",
    "bullet3.wav",
    "missile.wav",
    "rocket.wav",
    "explode.wav",
    "enemy1_down.wav",
    "enemy2_down.wav",
    "enemy3_down.wav",
    "me_down.wav",
    "gift_disappear1.wav",
    "gift_disappear2.wav",
    "FullFirePower.wav",
    "CenturyShield.wav",
    "AllMyPeople.wav",
});
static_assert(kSoundPaths.size() == kSoundCount);

constexpr std::array<TextureId, 6> kBulletTextures = {
    TextureId::BulletEnemyCannon, TextureId::BulletPlayerCannon,
    TextureId::Missile,           TextureId::Rocket,
    TextureId::BulletPlayerSuper, TextureId::BulletBoss,
};

constexpr std::array<TextureId, 4> kScoutDeath = {
    TextureId::Enemy1Down1,
    TextureId::Enemy1Down2,
    TextureId::Enemy1Down3,
    TextureId::Enemy1Down4,
};
constexpr std::array<TextureId, 4> kSineDeath = {
    TextureId::Enemy2Down1,
    TextureId::Enemy2Down2,
    TextureId::Enemy2Down3,
    TextureId::Enemy2Down4,
};
constexpr std::array<TextureId, 6> kBossDeath = {
    TextureId::Enemy3Down1, TextureId::Enemy3Down2, TextureId::Enemy3Down3,
    TextureId::Enemy3Down4, TextureId::Enemy3Down5, TextureId::Enemy3Down6,
};
constexpr std::array<TextureId, 4> kPlayerDeath = {
    TextureId::PlayerDeath1,
    TextureId::PlayerDeath2,
    TextureId::PlayerDeath3,
    TextureId::PlayerDeath4,
};

} // namespace

std::string_view texturePath(TextureId texture) {
    return kTexturePaths.at(static_cast<std::size_t>(texture));
}

bool textureSmooth(TextureId texture) {
    return kTextureSmooth.at(static_cast<std::size_t>(texture));
}

std::string_view fontPath(FontId font) {
    return kFontPaths.at(static_cast<std::size_t>(font));
}

std::string_view soundPath(SoundId sound) {
    return kSoundPaths.at(static_cast<std::size_t>(sound));
}

std::string_view musicPath() {
    return "background.wav";
}

TextureId enemyTexture(EnemyType type) {
    switch (type) {
        case EnemyType::Scout: return TextureId::Enemy1;
        case EnemyType::Sine: return TextureId::Enemy2;
        case EnemyType::Boss: return TextureId::Enemy3;
    }
    return TextureId::Enemy1;
}

TextureId enemyHitTexture(EnemyType type) {
    switch (type) {
        case EnemyType::Scout: return TextureId::Enemy1Hit;
        case EnemyType::Sine: return TextureId::Enemy2Hit;
        case EnemyType::Boss: return TextureId::Enemy3Hit;
    }
    return TextureId::Enemy1Hit;
}

TextureId enemyDeathTexture(EnemyType type, int frame) {
    switch (type) {
        case EnemyType::Scout:
            return kScoutDeath.at(static_cast<std::size_t>(
                std::clamp(frame, 1, static_cast<int>(kScoutDeath.size())) - 1));
        case EnemyType::Sine:
            return kSineDeath.at(static_cast<std::size_t>(
                std::clamp(frame, 1, static_cast<int>(kSineDeath.size())) - 1));
        case EnemyType::Boss:
            return kBossDeath.at(static_cast<std::size_t>(
                std::clamp(frame, 1, static_cast<int>(kBossDeath.size())) - 1));
    }
    return TextureId::Enemy1;
}

TextureId bulletTexture(BulletVisual visual) {
    return kBulletTextures.at(static_cast<std::size_t>(visual));
}

TextureId giftTexture(GiftKind kind) {
    switch (kind) {
        case GiftKind::FullFirePower: return TextureId::GiftFullFirePower;
        case GiftKind::CenturyShield: return TextureId::GiftCenturyShield;
        case GiftKind::AllMyPeople: return TextureId::GiftAllMyPeople;
    }
    return TextureId::GiftFullFirePower;
}

TextureId playerDeathTexture(int frame) {
    return kPlayerDeath.at(
        static_cast<std::size_t>(std::clamp(frame, 1, static_cast<int>(kPlayerDeath.size())) - 1));
}

} // namespace thunder
