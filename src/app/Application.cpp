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

#include "thunder/app/Application.hpp"

#include "thunder/BuildConfig.hpp"
#include "thunder/core/Log.hpp"
#include "thunder/core/Random.hpp"
#include "thunder/game/Config.hpp"
#include "thunder/persistence/SaveCodec.hpp"

#include <algorithm>
#include <cstdlib>
#include <format>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

namespace thunder {
namespace {

bool validAssetsDirectory(const std::filesystem::path &path) {
    std::error_code error;
    return std::filesystem::is_directory(path, error) &&
           std::filesystem::is_regular_file(path / "background.png", error);
}

std::filesystem::path resolveAssetsDirectory(const ApplicationOptions &options) {
    if (options.assetsDirectory) {
        if (!validAssetsDirectory(*options.assetsDirectory)) {
            throw std::runtime_error("Assets directory is invalid: " +
                                     options.assetsDirectory->string());
        }
        return *options.assetsDirectory;
    }

    std::vector<std::filesystem::path> candidates;

    if (const char *environment = std::getenv("THUNDER_WINGS_ASSETS");
        environment != nullptr && *environment != '\0')
        candidates.emplace_back(environment);

    std::error_code error;
    if (!options.executablePath.empty()) {
        const auto executable = std::filesystem::absolute(options.executablePath, error);
        if (!error) {
            const auto directory = executable.parent_path();
            candidates.push_back(directory / "assets");
            candidates.push_back(directory / ".." / "share" / "thunder-wings" / "assets");
        }
    }

    candidates.push_back(std::filesystem::current_path(error) / "assets");
    candidates.emplace_back(build::kSourceAssetsDir);

    for (const auto &candidate : candidates) {
        if (validAssetsDirectory(candidate))
            return std::filesystem::weakly_canonical(candidate, error);
    }

    throw std::runtime_error(
        "Cannot find the assets directory. Use --assets-dir or set THUNDER_WINGS_ASSETS.");
}

} // namespace

Application::Application(ApplicationOptions options)
    : options_(options), assetsDirectory_(resolveAssetsDirectory(options_)),
      window_(sf::VideoMode({static_cast<unsigned int>(config::kScreenWidth),
                             static_cast<unsigned int>(config::kScreenHeight)}),
              "Thunder Wings", sf::Style::Titlebar | sf::Style::Close),
      assets_(assetsDirectory_, options_.audioEnabled && !options_.smokeTest),
      gameRenderer_(assets_), menuRenderer_(assets_) {
    window_.setFramerateLimit(config::kFramerateLimit);

    if (options_.saveFile) {
        saveRepository_.emplace(*options_.saveFile);
    } else if (const auto path = SaveRepository::defaultPath()) {
        saveRepository_.emplace(*path);
    } else {
        TW_LOG_WARNING("Could not determine a save path; saving is disabled");
    }

    TW_LOG_INFO("Assets directory: {}", assetsDirectory_.string());
    if (saveRepository_)
        TW_LOG_INFO("Save file: {}", saveRepository_->path().string());
    assets_.playMusic();
}

int Application::run() {
    if (options_.smokeTest) {
        runSmokeTest();
        return 0;
    }

    sf::Clock frameClock;
    while (window_.isOpen()) {
        const float deltaTime = std::min(frameClock.restart().asSeconds(), config::kMaxFrameDelta);
        processEvents();
        if (!window_.isOpen())
            break;
        update(deltaTime);
        render();
    }
    return 0;
}

void Application::processEvents() {
    while (const std::optional event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window_.close();
            return;
        }
        if (const auto *key = event->getIf<sf::Event::KeyPressed>())
            handleKey(key->code);
    }
}

void Application::handleKey(sf::Keyboard::Key key) {
    const auto isEnter = [key] { return key == sf::Keyboard::Key::Enter; };

    switch (screen_) {
        case Screen::Menu:
            if (key == sf::Keyboard::Key::Down || key == sf::Keyboard::Key::PageDown) {
                menuSelection_ = std::min(menuSelection_ + 1, kMenuOptionCount - 1);
            } else if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::PageUp) {
                menuSelection_ = std::max(menuSelection_ - 1, 0);
            } else if (isEnter()) {
                switch (menuSelection_) {
                    case 0: startNewGame(); break;
                    case 1: loadGame(); break;
                    case 2:
                        screen_ = Screen::Guide;
                        menuMessage_.clear();
                        break;
                    case 3:
                        screen_ = Screen::About;
                        menuMessage_.clear();
                        break;
                    case 4: window_.close(); break;
                    default: break;
                }
            }
            break;

        case Screen::Guide:
        case Screen::About:
            if (key == sf::Keyboard::Key::Escape)
                screen_ = Screen::Menu;
            break;

        case Screen::Playing:
            if (key == sf::Keyboard::Key::P)
                pauseGame();
            else if (key == sf::Keyboard::Key::Escape)
                enterMenu();
            break;

        case Screen::Paused:
            if (key == sf::Keyboard::Key::P) {
                resumeGame();
            } else if (key == sf::Keyboard::Key::Escape) {
                enterMenu();
            } else if (key == sf::Keyboard::Key::Down || key == sf::Keyboard::Key::PageDown) {
                pauseSelection_ = std::min(pauseSelection_ + 1, kPauseOptionCount - 1);
            } else if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::PageUp) {
                pauseSelection_ = std::max(pauseSelection_ - 1, 0);
            } else if (isEnter()) {
                switch (pauseSelection_) {
                    case 0: resumeGame(); break;
                    case 1: saveGame(); break;
                    case 2: enterMenu(); break;
                    default: break;
                }
            }
            break;

        case Screen::GameOver:
            if (key == sf::Keyboard::Key::Escape)
                enterMenu();
            break;
    }
}

void Application::update(float deltaTime) {
    switch (screen_) {
        case Screen::Menu:
        case Screen::Guide:
        case Screen::About: break;

        case Screen::Playing:
            if (!world_)
                return;
            accumulator_ += deltaTime;
            accumulator_ = std::min(accumulator_, config::kMaxFrameDelta);
            while (accumulator_ >= config::kFixedTimeStep && screen_ == Screen::Playing) {
                world_->update(config::kFixedTimeStep, currentInput());
                drainSounds();
                accumulator_ -= config::kFixedTimeStep;
                if (world_->playerDefeated()) {
                    screen_ = Screen::GameOver;
                    gameOverTimer_ = 0.0f;
                    accumulator_ = 0.0f;
                    break;
                }
            }
            break;

        case Screen::Paused: break;

        case Screen::GameOver:
            gameOverTimer_ += deltaTime;
            if (gameOverTimer_ >= config::kGameOverDelay)
                enterMenu();
            break;
    }
}

void Application::render() {
    window_.clear(sf::Color::Black);

    switch (screen_) {
        case Screen::Menu:
            menuRenderer_.drawMenu(window_, menuSelection_,
                                   saveRepository_ && saveRepository_->exists(), menuMessage_);
            break;
        case Screen::Guide: menuRenderer_.drawPage(window_, guideLines()); break;
        case Screen::About: menuRenderer_.drawPage(window_, aboutLines()); break;
        case Screen::Playing:
            if (world_)
                gameRenderer_.draw(window_, *world_);
            break;
        case Screen::Paused:
            if (world_)
                gameRenderer_.draw(window_, *world_);
            gameRenderer_.drawPauseOverlay(window_, pauseSelection_, pauseMessage_);
            break;
        case Screen::GameOver:
            if (world_)
                gameRenderer_.draw(window_, *world_);
            gameRenderer_.drawGameOver(window_);
            break;
    }

    window_.display();
}

void Application::startNewGame() {
    world_.emplace();
    screen_ = Screen::Playing;
    accumulator_ = 0.0f;
    gameOverTimer_ = 0.0f;
    menuMessage_.clear();
    pauseMessage_.clear();
    assets_.playMusic();
}

void Application::loadGame() {
    if (!saveRepository_ || !saveRepository_->exists()) {
        menuMessage_ = "No saved game found";
        return;
    }

    std::string error;
    const auto save = saveRepository_->load(error);
    if (!save) {
        menuMessage_ = "Load failed: " + error;
        TW_LOG_WARNING("Load failed: {}", error);
        return;
    }

    world_.emplace(GameWorld::restore(save->world));
    screen_ = Screen::Playing;
    accumulator_ = 0.0f;
    gameOverTimer_ = 0.0f;
    menuMessage_.clear();
    pauseMessage_.clear();
    assets_.playMusic();
    TW_LOG_INFO("Loaded save from {}", saveRepository_->path().string());
}

void Application::saveGame() {
    if (!world_)
        return;
    if (!saveRepository_) {
        pauseMessage_ = "Saving is unavailable";
        return;
    }

    SaveGame game;
    game.world = world_->snapshot();
    std::string error;
    if (saveRepository_->save(game, error)) {
        pauseMessage_ = "Saved";
        TW_LOG_INFO("Saved game to {}", saveRepository_->path().string());
    } else {
        pauseMessage_ = "Save failed: " + error;
        TW_LOG_ERROR("Save failed: {}", error);
    }
}

void Application::enterMenu() {
    screen_ = Screen::Menu;
    world_.reset();
    accumulator_ = 0.0f;
    gameOverTimer_ = 0.0f;
    menuSelection_ = 0;
    menuMessage_.clear();
    pauseMessage_.clear();
    assets_.playMusic();
}

void Application::pauseGame() {
    if (screen_ != Screen::Playing)
        return;
    screen_ = Screen::Paused;
    pauseSelection_ = 0;
    pauseMessage_.clear();
    accumulator_ = 0.0f;
    assets_.pauseMusic();
    assets_.playSound(SoundId::Pause);
}

void Application::resumeGame() {
    if (screen_ != Screen::Paused)
        return;
    screen_ = Screen::Playing;
    accumulator_ = 0.0f;
    pauseMessage_.clear();
    assets_.resumeMusic();
}

void Application::drainSounds() {
    if (!world_)
        return;
    for (const SoundId sound : world_->takeSounds())
        assets_.playSound(sound);
}

PlayerInput Application::currentInput() const {
    PlayerInput input;
    input.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
    input.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
    input.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
    input.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    return input;
}

std::span<const std::string> Application::guideLines() const {
    static const std::vector<std::string> lines = [] {
        return std::vector<std::string>{
            "-- CONTROLS --",
            "Move: Arrow Keys     Pause: P     Menu: Esc",
            "",
            "-- PLAYER --",
            "Fire: 2 bullets @ 6144 dmg every 0.144s",
            "Heal: +128 HP every 0.64s",
            "Lifesteal: +5000~10000 HP on normal kill",
            "           +100% boss HP on boss kill",
            "Max HP: 1,024,000",
            "",
            "-- ENEMIES --",
            "Enemy I    HP 2048   Speed 128-256   Dmg 512",
            "Enemy II   HP 16,384 Speed 64-256    Dmg 728 (sine-wave motion)",
            "Boss III   HP 840k-1200k  Circles top of screen",
            "           Heal: +1000 HP every 1.0s",
            "           Volley: 6 bullets (6*4096)",
            "           Missiles: 17,203 dmg each",
            "           Rockets: 6,553.6 dmg min or up to 18.4% of your current HP",
            "",
            "-- GIFTS (every 32s, 36% chance) --",
            "FullFirePower: +400% fire rate",
            "CenturyShield: 90% damage reduction",
            "AllMyPeople: 16s, 256 allies/s advance and fight for you",
            "",
            "Press Esc to return to menu",
        };
    }();
    return lines;
}

std::span<const std::string> Application::aboutLines() const {
    static const std::vector<std::string> lines = [] {
        const std::string buildType =
            build::kBuildType.empty() ? std::string("unspecified") : std::string(build::kBuildType);
        return std::vector<std::string>{
            "-- ABOUT --",
            std::format("Thunder Wings {}", build::kVersion),
            "",
            "-- BUILD INFO --",
            std::format("Compiler: {}", build::kCompiler),
            std::format("Build type: {}", buildType),
            "Graphics/Audio: SFML 3",
            "JSON: nlohmann/json",
            "Tests: GoogleTest",
            "",
            "-- LEGAL --",
            std::string(build::kCopyright),
            "Licensed under the Apache License 2.0",
            "",
            "-- ACKNOWLEDGMENTS --",
            "Built with SFML 3 (Simple and Fast Multimedia Library)",
            "Special thanks to the open source community",
            "",
            "-- AUTHOR --",
            "Nuo Shen",
            "",
            "Press Esc to return to menu",
        };
    }();
    return lines;
}

void Application::runSmokeTest() {
    screen_ = Screen::Menu;
    render();

    world_.emplace(0x5EED1234ULL);
    world_->grantGift(GiftKind::AllMyPeople);
    for (int i = 0; i < 240; ++i) {
        world_->update(config::kFixedTimeStep, PlayerInput{});
        drainSounds();
    }
    screen_ = Screen::Playing;
    render();

    if (world_->alliesSummoned() < 512U || world_->allies().empty())
        throw std::runtime_error("Smoke test failed: AllMyPeople did not summon allies");

    TW_LOG_INFO("Smoke test passed: {} allies summoned, {} allies on screen",
                world_->alliesSummoned(), world_->allies().size());
    window_.close();
}

} // namespace thunder
