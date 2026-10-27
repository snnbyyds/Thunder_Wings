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

#include <array>
#include <cstdint>

namespace thunder {

/// Serializable 256-bit RNG state (xoshiro256**).
struct RandomState {
    std::array<std::uint64_t, 4> words{};

    constexpr bool operator==(const RandomState &) const = default;
};

[[nodiscard]] std::uint64_t makeSeed();

class Random {
public:
    explicit Random(std::uint64_t seed = makeSeed());

    void seed(std::uint64_t value);

    [[nodiscard]] std::uint64_t nextU64();

    /// Uniform value in [minimum, maximum). Returns minimum when the range is empty.
    [[nodiscard]] float uniform(float minimum, float maximum);

    /// Uniform integer in [minimum, maximum] (both inclusive).
    [[nodiscard]] int uniformInt(int minimum, int maximum);

    /// True with probability p; p is clamped to [0, 1].
    [[nodiscard]] bool chance(float probability);

    template <std::size_t N>
    [[nodiscard]] std::size_t weightedIndex(const std::array<float, N> &weights) {
        static_assert(N > 0);
        float total = 0.0f;
        for (const float weight : weights)
            total += weight > 0.0f ? weight : 0.0f;
        if (total <= 0.0f)
            return 0;

        float roll = uniform(0.0f, total);
        float cumulative = 0.0f;
        for (std::size_t i = 0; i < N; ++i) {
            const float weight = weights[i] > 0.0f ? weights[i] : 0.0f;
            cumulative += weight;
            if (roll < cumulative)
                return i;
        }
        return N - 1;
    }

    [[nodiscard]] RandomState state() const {
        return s_;
    }

    void setState(const RandomState &state);

private:
    static std::uint64_t splitMix64(std::uint64_t &state);
    static std::uint64_t rotateLeft(std::uint64_t value, int amount);

    RandomState s_{};
};

} // namespace thunder
