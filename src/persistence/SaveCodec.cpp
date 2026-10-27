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

#include "thunder/persistence/SaveCodec.hpp"

#include "thunder/game/Config.hpp"

#include <nlohmann/json.hpp>

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

namespace thunder {
namespace {

using Json = nlohmann::json;

constexpr std::size_t kMaxBullets = 8192;
constexpr std::size_t kMaxEnemies = 512;
constexpr std::size_t kMaxAllies = 1024;
constexpr std::size_t kMaxGifts = 16;

[[noreturn]] void invalid(const std::string &message) {
    throw SaveError(message);
}

const Json &objectField(const Json &object, const char *name) {
    if (!object.contains(name) || !object.at(name).is_object())
        invalid(std::string("field '") + name + "' must be an object");
    return object.at(name);
}

const Json &arrayField(const Json &object, const char *name, std::size_t maximum) {
    if (!object.contains(name) || !object.at(name).is_array())
        invalid(std::string("field '") + name + "' must be an array");
    const Json &array = object.at(name);
    if (array.size() > maximum)
        invalid(std::string("field '") + name + "' has too many entries");
    return array;
}

float readFloat(const Json &object, const char *name) {
    if (!object.contains(name) || !object.at(name).is_number())
        invalid(std::string("field '") + name + "' must be a number");
    const double value = object.at(name).get<double>();
    if (!std::isfinite(value))
        invalid(std::string("field '") + name + "' must be finite");
    return static_cast<float>(value);
}

float readNonNegativeFloat(const Json &object, const char *name) {
    const float value = readFloat(object, name);
    if (value < 0.0f)
        invalid(std::string("field '") + name + "' must be non-negative");
    return value;
}

int readInt(const Json &object, const char *name, int minimum, int maximum) {
    if (!object.contains(name) || !object.at(name).is_number_integer())
        invalid(std::string("field '") + name + "' must be an integer");
    const std::int64_t value = object.at(name).get<std::int64_t>();
    if (value < minimum || value > maximum)
        invalid(std::string("field '") + name + "' is out of range");
    return static_cast<int>(value);
}

std::uint64_t readU64(const Json &object, const char *name) {
    if (!object.contains(name) || !object.at(name).is_number_integer())
        invalid(std::string("field '") + name + "' must be an integer");
    if (!object.at(name).is_number_unsigned() && object.at(name).get<std::int64_t>() < 0)
        invalid(std::string("field '") + name + "' must be non-negative");
    return object.at(name).get<std::uint64_t>();
}

bool readBool(const Json &object, const char *name) {
    if (!object.contains(name) || !object.at(name).is_boolean())
        invalid(std::string("field '") + name + "' must be a boolean");
    return object.at(name).get<bool>();
}

std::string readString(const Json &object, const char *name) {
    if (!object.contains(name) || !object.at(name).is_string())
        invalid(std::string("field '") + name + "' must be a string");
    return object.at(name).get<std::string>();
}

Vec2 readVec2(const Json &object, const char *name) {
    if (!object.contains(name) || !object.at(name).is_array() || object.at(name).size() != 2U)
        invalid(std::string("field '") + name + "' must be a two-element array");
    const Json &array = object.at(name);
    for (const auto &value : array) {
        if (!value.is_number())
            invalid(std::string("field '") + name + "' must contain numbers");
    }
    const Vec2 result{array[0].get<float>(), array[1].get<float>()};
    if (!std::isfinite(result.x) || !std::isfinite(result.y))
        invalid(std::string("field '") + name + "' must be finite");
    return result;
}

EnemyType parseEnemyType(const std::string &name) {
    if (name == "Scout")
        return EnemyType::Scout;
    if (name == "Sine")
        return EnemyType::Sine;
    if (name == "Boss")
        return EnemyType::Boss;
    invalid("unknown enemy type '" + name + "'");
}

BulletKind parseBulletKind(const std::string &name) {
    if (name == "Cannon")
        return BulletKind::Cannon;
    if (name == "Missile")
        return BulletKind::Missile;
    if (name == "Rocket")
        return BulletKind::Rocket;
    invalid("unknown bullet kind '" + name + "'");
}

BulletVisual parseBulletVisual(const std::string &name) {
    if (name == "EnemyCannon")
        return BulletVisual::EnemyCannon;
    if (name == "PlayerCannon")
        return BulletVisual::PlayerCannon;
    if (name == "Missile")
        return BulletVisual::Missile;
    if (name == "Rocket")
        return BulletVisual::Rocket;
    if (name == "PlayerSuper")
        return BulletVisual::PlayerSuper;
    if (name == "BossBullet")
        return BulletVisual::BossBullet;
    invalid("unknown bullet visual '" + name + "'");
}

GiftKind parseGiftKind(const std::string &name) {
    if (name == "FullFirePower")
        return GiftKind::FullFirePower;
    if (name == "CenturyShield")
        return GiftKind::CenturyShield;
    if (name == "AllMyPeople")
        return GiftKind::AllMyPeople;
    invalid("unknown gift kind '" + name + "'");
}

Json vec2Json(Vec2 value) {
    return Json::array({value.x, value.y});
}

Json giftToJson(const GiftState &gift) {
    return Json{{"kind", giftKindName(gift.kind)},
                {"remaining", gift.remaining},
                {"maxTime", gift.maxTime},
                {"warning1", gift.warning1Played},
                {"warning2", gift.warning2Played},
                {"summonTimer", gift.summonTimer},
                {"summonsRemaining", gift.summonsRemaining}};
}

GiftState giftFromJson(const Json &value) {
    GiftState gift;
    if (!value.is_object())
        invalid("gift entry must be an object");
    gift.kind = parseGiftKind(readString(value, "kind"));
    gift.remaining = readNonNegativeFloat(value, "remaining");
    gift.maxTime = readNonNegativeFloat(value, "maxTime");
    gift.warning1Played = readBool(value, "warning1");
    gift.warning2Played = readBool(value, "warning2");
    gift.summonTimer = readNonNegativeFloat(value, "summonTimer");
    gift.summonsRemaining = readInt(value, "summonsRemaining", 0, config::kAllMyPeopleSummons);
    return gift;
}

Json playerToJson(const PlayerState &player) {
    Json gifts = Json::array();
    for (const auto &gift : player.gifts)
        gifts.push_back(giftToJson(gift));

    return Json{{"position", vec2Json(player.position)},
                {"health", player.health},
                {"shotCooldown", player.shotCooldown},
                {"regenCooldown", player.regenCooldown},
                {"animationTimer", player.animationTimer},
                {"textureIndex", player.textureIndex},
                {"dying", player.dying},
                {"deathFrame", player.deathFrame},
                {"deathTimer", player.deathTimer},
                {"dead", player.dead},
                {"shotCount", player.shotCount},
                {"gifts", std::move(gifts)}};
}

PlayerState playerFromJson(const Json &value) {
    if (!value.is_object())
        invalid("player must be an object");
    PlayerState player;
    player.position = readVec2(value, "position");
    player.health = readNonNegativeFloat(value, "health");
    player.shotCooldown = readFloat(value, "shotCooldown");
    player.regenCooldown = readFloat(value, "regenCooldown");
    player.animationTimer = readNonNegativeFloat(value, "animationTimer");
    player.textureIndex = readInt(value, "textureIndex", 0, 1);
    player.dying = readBool(value, "dying");
    player.deathFrame = readInt(value, "deathFrame", 0, 4);
    player.deathTimer = readNonNegativeFloat(value, "deathTimer");
    player.dead = readBool(value, "dead");
    const std::uint64_t shotCount = readU64(value, "shotCount");
    if (shotCount > std::numeric_limits<std::uint32_t>::max())
        invalid("shotCount is out of range");
    player.shotCount = static_cast<std::uint32_t>(shotCount);

    const Json &gifts = arrayField(value, "gifts", kMaxGifts);
    player.gifts.reserve(gifts.size());
    for (const auto &entry : gifts)
        player.gifts.push_back(giftFromJson(entry));
    return player;
}

Json bulletToJson(const BulletState &bullet) {
    return Json{{"kind", bulletKindName(bullet.kind)},
                {"visual", bulletVisualName(bullet.visual)},
                {"fromPlayer", bullet.fromPlayer},
                {"position", vec2Json(bullet.position)},
                {"direction", vec2Json(bullet.direction)},
                {"speed", bullet.speed},
                {"damage", bullet.damage},
                {"damageRate", bullet.damageRate},
                {"tracking", bullet.tracking},
                {"age", bullet.age}};
}

BulletState bulletFromJson(const Json &value) {
    if (!value.is_object())
        invalid("bullet entry must be an object");
    BulletState bullet;
    bullet.kind = parseBulletKind(readString(value, "kind"));
    bullet.visual = parseBulletVisual(readString(value, "visual"));
    bullet.fromPlayer = readBool(value, "fromPlayer");
    bullet.position = readVec2(value, "position");
    bullet.direction = readVec2(value, "direction");
    bullet.speed = readNonNegativeFloat(value, "speed");
    bullet.damage = readNonNegativeFloat(value, "damage");
    bullet.damageRate = readNonNegativeFloat(value, "damageRate");
    bullet.tracking = readNonNegativeFloat(value, "tracking");
    bullet.age = readNonNegativeFloat(value, "age");
    return bullet;
}

Json enemyToJson(const EnemyState &enemy) {
    return Json{{"type", enemyTypeName(enemy.type)},
                {"position", vec2Json(enemy.position)},
                {"health", enemy.health},
                {"maxHealth", enemy.maxHealth},
                {"speed", enemy.speed},
                {"bulletSpeed", enemy.bulletSpeed},
                {"shotDamage", enemy.shotDamage},
                {"killBonus", enemy.killBonus},
                {"shotCooldown", enemy.shotCooldown},
                {"shotGap", enemy.shotGap},
                {"shotIndex", enemy.shotIndex},
                {"motionTime", enemy.motionTime},
                {"motionCenterX", enemy.motionCenterX},
                {"motionAmplitude", enemy.motionAmplitude},
                {"motionFrequency", enemy.motionFrequency},
                {"hitFlash", enemy.hitFlash},
                {"dying", enemy.dying},
                {"deathFrame", enemy.deathFrame},
                {"deathTimer", enemy.deathTimer},
                {"rewarded", enemy.rewarded},
                {"recoverRate", enemy.recoverRate}};
}

EnemyState enemyFromJson(const Json &value) {
    if (!value.is_object())
        invalid("enemy entry must be an object");
    EnemyState enemy;
    enemy.type = parseEnemyType(readString(value, "type"));
    enemy.position = readVec2(value, "position");
    enemy.health = readNonNegativeFloat(value, "health");
    enemy.maxHealth = readFloat(value, "maxHealth");
    if (enemy.maxHealth <= 0.0f)
        invalid("enemy maxHealth must be positive");
    enemy.speed = readNonNegativeFloat(value, "speed");
    enemy.bulletSpeed = readNonNegativeFloat(value, "bulletSpeed");
    enemy.shotDamage = readNonNegativeFloat(value, "shotDamage");
    enemy.killBonus = readNonNegativeFloat(value, "killBonus");
    enemy.shotCooldown = readFloat(value, "shotCooldown");
    enemy.shotGap = readNonNegativeFloat(value, "shotGap");
    enemy.shotIndex = readInt(value, "shotIndex", 0, 1000);
    enemy.motionTime = readNonNegativeFloat(value, "motionTime");
    enemy.motionCenterX = readFloat(value, "motionCenterX");
    enemy.motionAmplitude = readNonNegativeFloat(value, "motionAmplitude");
    enemy.motionFrequency = readNonNegativeFloat(value, "motionFrequency");
    enemy.hitFlash = readNonNegativeFloat(value, "hitFlash");
    enemy.dying = readBool(value, "dying");
    enemy.deathFrame = readInt(value, "deathFrame", 0, 6);
    enemy.deathTimer = readNonNegativeFloat(value, "deathTimer");
    enemy.rewarded = readBool(value, "rewarded");
    enemy.recoverRate = readNonNegativeFloat(value, "recoverRate");
    return enemy;
}

Json allyToJson(const AllyState &ally) {
    return Json{{"position", vec2Json(ally.position)},
                {"health", ally.health},
                {"fireCooldown", ally.fireCooldown}};
}

AllyState allyFromJson(const Json &value) {
    if (!value.is_object())
        invalid("ally entry must be an object");
    AllyState ally;
    ally.position = readVec2(value, "position");
    ally.health = readNonNegativeFloat(value, "health");
    ally.fireCooldown = readFloat(value, "fireCooldown");
    return ally;
}

Json worldToJson(const WorldState &world) {
    Json enemies = Json::array();
    for (const auto &enemy : world.enemies)
        enemies.push_back(enemyToJson(enemy));

    Json bullets = Json::array();
    for (const auto &bullet : world.bullets)
        bullets.push_back(bulletToJson(bullet));

    Json allies = Json::array();
    for (const auto &ally : world.allies)
        allies.push_back(allyToJson(ally));

    return Json{{"elapsed", world.elapsed},
                {"kills", world.kills},
                {"alliesSummoned", world.alliesSummoned},
                {"rng", Json::array({world.random.words[0], world.random.words[1],
                                     world.random.words[2], world.random.words[3]})},
                {"enemySpawnTimer", world.enemySpawnTimer},
                {"giftSpawnTimer", world.giftSpawnTimer},
                {"player", playerToJson(world.player)},
                {"enemies", std::move(enemies)},
                {"bullets", std::move(bullets)},
                {"allies", std::move(allies)}};
}

WorldState worldFromJson(const Json &value) {
    if (!value.is_object())
        invalid("world must be an object");

    WorldState world;
    world.elapsed = readNonNegativeFloat(value, "elapsed");
    world.kills = readU64(value, "kills");
    world.alliesSummoned = readU64(value, "alliesSummoned");
    world.enemySpawnTimer = readNonNegativeFloat(value, "enemySpawnTimer");
    world.giftSpawnTimer = readNonNegativeFloat(value, "giftSpawnTimer");

    const Json &rng = arrayField(value, "rng", 4);
    if (rng.size() != 4U)
        invalid("field 'rng' must contain four words");
    for (std::size_t i = 0; i < 4; ++i) {
        if (!rng[i].is_number_integer())
            invalid("rng word must be an integer");
        if (!rng[i].is_number_unsigned() && rng[i].get<std::int64_t>() < 0)
            invalid("rng word must be non-negative");
        world.random.words[i] = rng[i].get<std::uint64_t>();
    }

    world.player = playerFromJson(objectField(value, "player"));

    const Json &enemies = arrayField(value, "enemies", kMaxEnemies);
    world.enemies.reserve(enemies.size());
    for (const auto &entry : enemies)
        world.enemies.push_back(enemyFromJson(entry));

    const Json &bullets = arrayField(value, "bullets", kMaxBullets);
    world.bullets.reserve(bullets.size());
    for (const auto &entry : bullets)
        world.bullets.push_back(bulletFromJson(entry));

    const Json &allies = arrayField(value, "allies", kMaxAllies);
    world.allies.reserve(allies.size());
    for (const auto &entry : allies)
        world.allies.push_back(allyFromJson(entry));

    return world;
}

} // namespace

std::string encodeSave(const SaveGame &game) {
    Json root{{"format", "thunder-wings-save"},
              {"version", game.version},
              {"world", worldToJson(game.world)}};
    return root.dump(2);
}

SaveGame decodeSave(std::string_view text) {
    try {
        const Json root = Json::parse(text.begin(), text.end());
        if (!root.is_object())
            invalid("invalid or corrupted save file");
        if (readString(root, "format") != "thunder-wings-save")
            invalid("invalid or corrupted save file");

        const int version = readInt(root, "version", 0, 1000);
        if (version != SaveGame::kVersion)
            invalid("invalid or corrupted save file");

        SaveGame game;
        game.version = version;
        game.world = worldFromJson(objectField(root, "world"));
        return game;
    } catch (const SaveError &) { throw; } catch (const std::exception &) {
        throw SaveError("invalid or corrupted save file");
    }
}

} // namespace thunder
