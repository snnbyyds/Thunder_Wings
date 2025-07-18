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
#include "../Core/ISerializable.hpp"
#include <SFML/Graphics.hpp>

class Entity : public ISerializable {
public:
    Entity() = default;
    virtual ~Entity() = default;

    virtual void update(float deltaTime) {};
    virtual void render(sf::RenderWindow &window);
    virtual sf::FloatRect getBounds() const;

    sf::Vector2f getPosition();

    virtual boost::json::object serialize() const override;
    virtual void deserialize(const boost::json::object &o) override;

    bool isAvailable() const { return avail; }

    void setAvailable(bool available) { avail = available; }

protected:
    bool avail = true;
    sf::Sprite sprite;
};