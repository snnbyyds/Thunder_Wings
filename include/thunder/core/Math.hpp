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

#include <algorithm>
#include <cmath>
#include <numbers>

namespace thunder {

struct Vec2 {
    float x{};
    float y{};

    constexpr Vec2 operator+(const Vec2 &other) const {
        return {x + other.x, y + other.y};
    }

    constexpr Vec2 operator-(const Vec2 &other) const {
        return {x - other.x, y - other.y};
    }

    constexpr Vec2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    constexpr Vec2 operator/(float scalar) const {
        return {x / scalar, y / scalar};
    }

    constexpr Vec2 &operator+=(const Vec2 &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2 &operator-=(const Vec2 &other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr bool operator==(const Vec2 &other) const = default;
};

constexpr Vec2 operator*(float scalar, const Vec2 &value) {
    return value * scalar;
}

constexpr float lengthSquared(Vec2 value) {
    return value.x * value.x + value.y * value.y;
}

inline float length(Vec2 value) {
    return std::sqrt(lengthSquared(value));
}

constexpr float dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

inline Vec2 normalized(Vec2 value, Vec2 fallback = {}) {
    const float magnitude = length(value);
    if (magnitude <= 1.0e-6f)
        return fallback;
    return value / magnitude;
}

inline float angleOf(Vec2 value) {
    return std::atan2(value.y, value.x);
}

inline Vec2 fromAngle(float radians) {
    return {std::cos(radians), std::sin(radians)};
}

inline float normalizeAngle(float radians) {
    return std::remainder(radians, 2.0f * std::numbers::pi_v<float>);
}

inline float lerp(float from, float to, float t) {
    return from + t * (to - from);
}

inline Vec2 lerp(Vec2 from, Vec2 to, float t) {
    return from + (to - from) * t;
}

inline float clamp(float value, float minimum, float maximum) {
    return std::clamp(value, minimum, maximum);
}

struct Aabb {
    Vec2 position{};
    Vec2 size{};

    static Aabb fromCenter(Vec2 center, Vec2 size) {
        return {center - size * 0.5f, size};
    }

    [[nodiscard]] Vec2 center() const {
        return position + size * 0.5f;
    }

    [[nodiscard]] bool intersects(const Aabb &other) const {
        return position.x < other.position.x + other.size.x &&
               position.x + size.x > other.position.x &&
               position.y < other.position.y + other.size.y &&
               position.y + size.y > other.position.y;
    }

    [[nodiscard]] bool contains(Vec2 point) const {
        return point.x >= position.x && point.x <= position.x + size.x && point.y >= position.y &&
               point.y <= position.y + size.y;
    }
};

} // namespace thunder
