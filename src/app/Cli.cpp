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

#include "thunder/app/Cli.hpp"

#include <string_view>

namespace thunder {
namespace {

std::string requireValue(int argc, char *argv[], int &index) {
    if (index + 1 >= argc)
        throw CliError(std::string("Missing value for ") + argv[index]);
    ++index;
    return argv[index];
}

} // namespace

CliOptions parseCommandLine(int argc, char *argv[]) {
    CliOptions options;
    if (argc > 0)
        options.executablePath = argv[0];

    for (int i = 1; i < argc; ++i) {
        const std::string_view argument = argv[i];
        if (argument == "--help" || argument == "-h") {
            options.showHelp = true;
        } else if (argument == "--version" || argument == "-v") {
            options.showVersion = true;
        } else if (argument == "--no-audio") {
            options.noAudio = true;
        } else if (argument == "--smoke-test") {
            options.smokeTest = true;
        } else if (argument == "--assets-dir") {
            options.assetsDirectory = requireValue(argc, argv, i);
        } else if (argument == "--save-file") {
            options.saveFile = requireValue(argc, argv, i);
        } else {
            throw CliError("Unknown option: " + std::string(argument));
        }
    }

    if (options.smokeTest)
        options.noAudio = true;
    return options;
}

std::string usageText() {
    return R"(Thunder Wings
Usage: thunder_wings [options]

Options:
  -h, --help             Show this help text
  -v, --version          Show version and build information
      --no-audio         Disable music and sound effects
      --smoke-test       Load assets, render a short deterministic game and exit
      --assets-dir PATH  Override the assets directory
      --save-file PATH   Override the save file location
)";
}

} // namespace thunder
