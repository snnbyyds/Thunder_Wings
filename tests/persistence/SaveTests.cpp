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
#include "thunder/persistence/SaveCodec.hpp"
#include "thunder/persistence/SaveRepository.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

void advance(thunder::GameWorld &world, float seconds) {
    const int steps = static_cast<int>(std::lround(seconds / thunder::config::kFixedTimeStep));
    for (int i = 0; i < steps; ++i)
        world.update(thunder::config::kFixedTimeStep, thunder::PlayerInput{});
}

std::filesystem::path makeTemporaryDirectory() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto directory =
        std::filesystem::temp_directory_path() / ("thunder-wings-test-" + std::to_string(stamp));
    std::filesystem::create_directories(directory);
    return directory;
}

} // namespace

TEST(SaveTest, CodecRoundTripAndDeterministicContinuation) {
    thunder::GameWorld world(42);
    world.grantGift(thunder::GiftKind::AllMyPeople);
    advance(world, 3.0f);

    thunder::SaveGame game;
    game.world = world.snapshot();
    const std::string encoded = thunder::encodeSave(game);
    const thunder::SaveGame decoded = thunder::decodeSave(encoded);
    EXPECT_EQ(decoded.version, thunder::SaveGame::kVersion);
    EXPECT_EQ(thunder::encodeSave(decoded), encoded);

    thunder::GameWorld restored = thunder::GameWorld::restore(decoded.world);
    advance(world, 2.0f);
    advance(restored, 2.0f);

    thunder::SaveGame afterOriginal;
    afterOriginal.world = world.snapshot();
    thunder::SaveGame afterRestored;
    afterRestored.world = restored.snapshot();
    EXPECT_EQ(thunder::encodeSave(afterOriginal), thunder::encodeSave(afterRestored));
}

TEST(SaveTest, RepositoryWritesLoadsAndRecoversBackup) {
    const auto directory = makeTemporaryDirectory();
    const auto path = directory / "game_data.json";
    const thunder::SaveRepository repository(path);

    thunder::SaveGame first;
    first.world = thunder::GameWorld(7).snapshot();
    first.world.kills = 0;

    std::string error;
    ASSERT_TRUE(repository.save(first, error)) << error;
    EXPECT_TRUE(repository.exists());

    auto loaded = repository.load(error);
    ASSERT_TRUE(loaded) << error;
    EXPECT_EQ(thunder::encodeSave(*loaded), thunder::encodeSave(first));
    EXPECT_FALSE(std::filesystem::exists(path.string() + ".tmp"));

    // The second save rotates the previous good file into .bak.
    first.world.kills = 99;
    ASSERT_TRUE(repository.save(first, error)) << error;
    {
        std::ofstream corrupted(path, std::ios::binary | std::ios::trunc);
        corrupted << "not json";
    }

    loaded = repository.load(error);
    ASSERT_TRUE(loaded) << error;
    EXPECT_EQ(loaded->world.kills, 0U);
    EXPECT_FALSE(std::filesystem::exists(path.string() + ".tmp"));

    std::filesystem::remove_all(directory);
}

TEST(SaveTest, RepositoryUsesAGenericErrorForOldOrCorruptFiles) {
    const auto directory = makeTemporaryDirectory();
    const auto path = directory / "game_data.json";
    const thunder::SaveRepository repository(path);

    {
        std::ofstream oldSave(path, std::ios::binary | std::ios::trunc);
        oldSave << R"({"format":"thunder-wings-save","version":1,"world":{}})";
    }
    std::string error;
    EXPECT_FALSE(repository.load(error));
    EXPECT_EQ(error, "Invalid or corrupted save file");

    {
        std::ofstream corruptSave(path, std::ios::binary | std::ios::trunc);
        corruptSave << "not json";
    }
    EXPECT_FALSE(repository.load(error));
    EXPECT_EQ(error, "Invalid or corrupted save file");

    std::filesystem::remove_all(directory);
}

TEST(SaveTest, RejectsInvalidAndCorruptFiles) {
    EXPECT_THROW((void)thunder::decodeSave("{"), thunder::SaveError);
    EXPECT_THROW(
        (void)thunder::decodeSave(R"({"format":"thunder-wings-save","version":1,"world":{}})"),
        thunder::SaveError);
    EXPECT_THROW((void)thunder::decodeSave(R"({"format":"other","version":2,"world":{}})"),
                 thunder::SaveError);
    EXPECT_THROW((void)thunder::decodeSave(R"({"format":"thunder-wings-save","version":2})"),
                 thunder::SaveError);

    thunder::GameWorld bulletWorld(1);
    advance(bulletWorld, 0.15f);
    thunder::SaveGame valid;
    valid.world = bulletWorld.snapshot();
    std::string encoded = thunder::encodeSave(valid);
    const std::string needle = R"("kind": "Cannon")";
    const auto position = encoded.find(needle);
    ASSERT_NE(position, std::string::npos);
    encoded.replace(position, needle.size(), R"("kind": "Laser")");
    EXPECT_THROW((void)thunder::decodeSave(encoded), thunder::SaveError);
}
