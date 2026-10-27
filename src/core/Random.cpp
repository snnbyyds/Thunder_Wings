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

#include "thunder/core/Random.hpp"

#include <chrono>
#include <random>

namespace thunder {

std::uint64_t makeSeed() {
    std::random_device device;
    const auto time = static_cast<std::uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    return (static_cast<std::uint64_t>(device()) << 32U) ^ device() ^ time;
}

std::uint64_t Random::splitMix64(std::uint64_t &state) {
    state += 0x9E3779B97F4A7C15ULL;
    std::uint64_t value = state;
    value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
    value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;
    return value ^ (value >> 31U);
}

std::uint64_t Random::rotateLeft(std::uint64_t value, int amount) {
    return (value << amount) | (value >> (64 - amount));
}

Random::Random(std::uint64_t seedValue) {
    seed(seedValue);
}

void Random::seed(std::uint64_t value) {
    std::uint64_t state = value;
    for (auto &word : s_.words)
        word = splitMix64(state);
}

std::uint64_t Random::nextU64() {
    const std::uint64_t result = rotateLeft(s_.words[1] * 5ULL, 7) * 9ULL;
    const std::uint64_t temporary = s_.words[1] << 17U;

    s_.words[2] ^= s_.words[0];
    s_.words[3] ^= s_.words[1];
    s_.words[1] ^= s_.words[2];
    s_.words[0] ^= s_.words[3];
    s_.words[2] ^= temporary;
    s_.words[3] = rotateLeft(s_.words[3], 45);
    return result;
}

void Random::setState(const RandomState &state) {
    s_ = state;
    // xoshiro256** locks at zero; make a damaged/hand-edited save recover.
    if (s_.words[0] == 0 && s_.words[1] == 0 && s_.words[2] == 0 && s_.words[3] == 0)
        seed(0x9E3779B97F4A7C15ULL);
}

float Random::uniform(float minimum, float maximum) {
    if (maximum <= minimum)
        return minimum;
    // 24 high-quality bits are plenty for a float in [0, 1).
    const float unit = static_cast<float>(nextU64() >> 40U) * (1.0f / 16777216.0f);
    return minimum + (maximum - minimum) * unit;
}

int Random::uniformInt(int minimum, int maximum) {
    if (maximum <= minimum)
        return minimum;
    const std::uint64_t span = static_cast<std::uint64_t>(maximum - minimum) + 1ULL;
    return minimum + static_cast<int>(nextU64() % span);
}

bool Random::chance(float probability) {
    if (probability <= 0.0f)
        return false;
    if (probability >= 1.0f)
        return true;
    return uniform(0.0f, 1.0f) < probability;
}

} // namespace thunder
