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

#include <gtest/gtest.h>

#include <filesystem>
#include <string_view>

#ifndef TW_SOURCE_ASSETS_DIR
#error "TW_SOURCE_ASSETS_DIR must be defined by the build"
#endif

TEST(AssetCatalogTest, EveryCatalogEntryExists) {
    const std::filesystem::path root(TW_SOURCE_ASSETS_DIR);

    for (std::size_t i = 0; i < static_cast<std::size_t>(thunder::TextureId::Count); ++i) {
        const auto id = static_cast<thunder::TextureId>(i);
        const auto path = thunder::texturePath(id);
        EXPECT_TRUE(std::filesystem::is_regular_file(root / path)) << path;
    }

    for (std::size_t i = 0; i < static_cast<std::size_t>(thunder::FontId::Count); ++i) {
        const auto path = thunder::fontPath(static_cast<thunder::FontId>(i));
        EXPECT_TRUE(std::filesystem::is_regular_file(root / path)) << path;
    }

    for (std::size_t i = 0; i < static_cast<std::size_t>(thunder::SoundId::Count); ++i) {
        const auto path = thunder::soundPath(static_cast<thunder::SoundId>(i));
        EXPECT_TRUE(std::filesystem::is_regular_file(root / path)) << path;
    }

    EXPECT_TRUE(std::filesystem::is_regular_file(root / thunder::musicPath()));
}

TEST(AssetCatalogTest, OnlyThreeGiftKindsRemain) {
    EXPECT_EQ(static_cast<int>(thunder::GiftKind::AllMyPeople) + 1, 3);
    EXPECT_EQ(thunder::giftKindName(thunder::GiftKind::AllMyPeople), "AllMyPeople");
}
