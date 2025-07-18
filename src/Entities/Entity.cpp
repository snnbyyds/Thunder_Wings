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

#include "Entities/Entity.hpp"

void Entity::render(sf::RenderWindow &window) {
    if (avail)
        window.draw(sprite);
}

sf::Vector2f Entity::getPosition() { return sprite.getPosition(); }

sf::FloatRect Entity::getBounds() const { return sprite.getGlobalBounds(); }

boost::json::object Entity::serialize() const {
    return {
        {"avail", true},
        {"position",
         {{"x", sprite.getPosition().x}, {"y", sprite.getPosition().y}}},
    };
}

void Entity::deserialize(const boost::json::object &o) {
    avail = o.at("avail").as_bool();
    auto dir_obj = o.at("position").as_object();
    float x = (float)dir_obj.at("x").as_double();
    float y = (float)dir_obj.at("y").as_double();
    sprite.setPosition(x, y);
}
