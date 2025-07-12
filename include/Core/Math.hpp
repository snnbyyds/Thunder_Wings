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
#include <SFML/System/Vector2.hpp>
#include <cmath>

class Math {
public:
    Math() = delete;
    Math(const Math &) = delete;
    Math &operator=(const Math &) = delete;

    // Mathematical constants
    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float DEG_TO_RAD = PI / 180.0f;
    static constexpr float RAD_TO_DEG = 180.0f / PI;

    // Calculate the angle (in radians) between two points
    static float angleBetween(const sf::Vector2f &a, const sf::Vector2f &b) {
        return std::atan2(b.y - a.y, b.x - a.x);
    }

    // Normalize a vector
    static sf::Vector2f normalize(const sf::Vector2f &v) {
        float length = magnitude(v);
        if (length > 0.0f)
            return v / length;
        return v;
    }

    // Calculate the length (magnitude) of a vector
    static float magnitude(const sf::Vector2f &v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    // Calculate the distance between two points
    static float distance(const sf::Vector2f &a, const sf::Vector2f &b) {
        return magnitude(b - a);
    }

    // Normalize an angle to be in the range [-PI, PI]
    static float normalizeAngle(float angle) {
        while (angle > PI)
            angle -= 2 * PI;
        while (angle < -PI)
            angle += 2 * PI;
        return angle;
    }

    // Linear interpolation between two values
    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    // Linear interpolation between two vectors
    static sf::Vector2f lerp(const sf::Vector2f &a, const sf::Vector2f &b,
                             float t) {
        return a + t * (b - a);
    }

    // Linear interpolation for angles (taking angle wrapping into account)
    static float lerpAngle(float a, float b, float t) {
        float diff = normalizeAngle(b - a);
        return a + t * diff;
    }

    // Dot product of two vectors
    static float dot(const sf::Vector2f &a, const sf::Vector2f &b) {
        return a.x * b.x + a.y * b.y;
    }

    // Convert an angle (in radians) to a direction vector
    static sf::Vector2f angleToVector(float angleRad) {
        return sf::Vector2f(std::cos(angleRad), std::sin(angleRad));
    }

    // Convert a direction vector to an angle (in radians)
    static float vectorToAngle(const sf::Vector2f &v) {
        return std::atan2(v.y, v.x);
    }
};
