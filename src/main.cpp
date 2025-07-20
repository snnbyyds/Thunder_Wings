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

#include "Core/Logging.hpp"
#include "Core/ResourceManager.hpp"
#include "Game/Menu.hpp"
#include <iostream>

static inline void printVersion() {
    // clang-format off
    std::cout << "thunder_wings " << TW_VERSION << "\n"
              << "Built on " << TW_BUILD_DATE << "\n"
              << "Compilation: " << TW_COMPILER_OPTIONS << "\n"
              << "Linking: " << TW_LINKER_OPTIONS << "\n"
              << TW_COPYRIGHT << "\n"
              << "Licensed under the " << TW_LICENSE << "\n"
              << "See https://www.apache.org/licenses/LICENSE-2.0\n"
              << std::endl;
    // clang-format on
}

int main(int argc, char *argv[]) {
    printVersion();
    if (argc > 1) {
        std::string_view arg = argv[1];
        if (arg == "--version" || arg == "-v")
            return EXIT_SUCCESS;
    }

    logging::init();
    LOG_INFO("Welcome!");

    try {
        Menu menu;
        menu.playLogo();
        menu.show();
    } catch (const TextureLoadException &e) {
        LOG_ERROR("Error: " << e.what());
        std::exit(EXIT_FAILURE);
    } catch (const FontLoadException &e) {
        LOG_ERROR("Error: " << e.what());
        std::exit(EXIT_FAILURE);
    } catch (const AudioLoadException &e) {
        LOG_ERROR("Error: " << e.what());
        std::exit(EXIT_FAILURE);
    } catch (const std::exception &e) {
        LOG_ERROR("Error: " << e.what());
        std::exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}