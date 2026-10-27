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

#include "thunder/core/Math.hpp"
#include "thunder/core/Random.hpp"

#include <gtest/gtest.h>

#include <array>

namespace {

using thunder::Aabb;
using thunder::Random;
using thunder::Vec2;

TEST(MathTest, NormalizesAndMeasuresVectors) {
    const Vec2 normalized = thunder::normalized({3.0f, 4.0f});
    EXPECT_NEAR(normalized.x, 0.6f, 1.0e-5f);
    EXPECT_NEAR(normalized.y, 0.8f, 1.0e-5f);
    EXPECT_NEAR(thunder::length(normalized), 1.0f, 1.0e-5f);
    EXPECT_EQ(thunder::normalized({0.0f, 0.0f}, {0.0f, -1.0f}), (Vec2{0.0f, -1.0f}));
}

TEST(MathTest, AabbIntersectionAndCenter) {
    const Aabb first = Aabb::fromCenter({10.0f, 10.0f}, {8.0f, 8.0f});
    const Aabb second{{13.0f, 13.0f}, {2.0f, 2.0f}};
    const Aabb outside{{30.0f, 30.0f}, {2.0f, 2.0f}};
    EXPECT_TRUE(first.intersects(second));
    EXPECT_FALSE(first.intersects(outside));
    EXPECT_EQ(first.center(), (Vec2{10.0f, 10.0f}));
    EXPECT_TRUE(first.contains({10.0f, 10.0f}));
}

TEST(RandomTest, IsDeterministicAndStateIsRestorable) {
    Random first(0x12345678ULL);
    Random second(0x12345678ULL);
    for (int i = 0; i < 20; ++i)
        EXPECT_EQ(first.nextU64(), second.nextU64());

    const auto saved = first.state();
    const auto expected = first.nextU64();
    first.setState(saved);
    EXPECT_EQ(first.nextU64(), expected);

    Random restored(0);
    restored.setState(saved);
    EXPECT_EQ(restored.state(), saved);
}

TEST(RandomTest, BoundsAndWeightedSelection) {
    Random random(99);
    for (int i = 0; i < 1000; ++i) {
        const float value = random.uniform(2.0f, 5.0f);
        EXPECT_GE(value, 2.0f);
        EXPECT_LT(value, 5.0f);
        const int integer = random.uniformInt(3, 7);
        EXPECT_GE(integer, 3);
        EXPECT_LE(integer, 7);
    }

    const std::array<float, 3> weights{0.0f, 0.0f, 1.0f};
    for (int i = 0; i < 100; ++i)
        EXPECT_EQ(random.weightedIndex(weights), 2U);

    EXPECT_FALSE(random.chance(0.0f));
    EXPECT_TRUE(random.chance(1.0f));
}

} // namespace
