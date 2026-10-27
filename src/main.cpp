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

#include "thunder/BuildConfig.hpp"
#include "thunder/app/Application.hpp"
#include "thunder/app/Cli.hpp"
#include "thunder/core/Log.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

void printVersion() {
    std::cout << "thunder_wings " << thunder::build::kVersion << '\n'
              << "Compiler: " << thunder::build::kCompiler << '\n'
              << "Build type: " << thunder::build::kBuildType << '\n'
              << thunder::build::kCopyright << '\n'
              << "Licensed under the " << thunder::build::kLicense << '\n';
}

void showFatalError(std::string_view message) {
#ifdef _WIN32
    MessageBoxA(nullptr, std::string(message).c_str(), "Thunder Wings", MB_ICONERROR | MB_OK);
#else
    std::cerr << message << '\n';
#endif
}

} // namespace

int main(int argc, char *argv[]) {
    try {
        const thunder::CliOptions cli = thunder::parseCommandLine(argc, argv);
        if (cli.showHelp) {
            std::cout << thunder::usageText();
            return 0;
        }
        if (cli.showVersion) {
            printVersion();
            return 0;
        }

        thunder::ApplicationOptions options;
        options.executablePath = cli.executablePath;
        options.assetsDirectory = cli.assetsDirectory;
        options.saveFile = cli.saveFile;
        options.audioEnabled = !cli.noAudio;
        options.smokeTest = cli.smokeTest;

        thunder::Application application(std::move(options));
        return application.run();
    } catch (const thunder::CliError &error) {
        std::cerr << error.what() << "\n\n" << thunder::usageText();
        return 2;
    } catch (const std::exception &error) {
        TW_LOG_ERROR("Fatal error: {}", error.what());
        showFatalError(std::string("Thunder Wings failed to start:\n") + error.what());
        return 1;
    }
}
