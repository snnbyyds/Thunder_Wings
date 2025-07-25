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

#include "Core/Timer.hpp"

Timer::Timer() : offset(0.0f) { clock.restart(); }

void Timer::restart() {
    offset = 0.0f;
    clock.restart();
}

float Timer::getElapsedTime() const {
    return clock.getElapsedTime().asSeconds() + offset;
}

void Timer::setElapsedTime(float seconds) {
    offset = seconds;
    clock.restart();
}

bool Timer::hasElapsed(float seconds) const {
    return getElapsedTime() >= seconds;
}