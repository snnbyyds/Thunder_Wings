# Thunder Wings

**Thunder Wings** is a modernized vertical-scrolling shooter built with **C++20** and **SFML 3**.
It keeps the original arcade gameplay and artwork while replacing the SFML 2 prototype architecture,
save system, build setup and test coverage.

[![Build and Test](https://github.com/snnbyyds/Thunder_Wings/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/snnbyyds/Thunder_Wings/actions/workflows/build.yml)

## Highlights

- **SFML 3 only** — no SFML 2 compatibility layer and no Boost dependency.
- **Layered architecture** — pure simulation in `thunder_core`; SFML window, assets, rendering and
  application flow live in the executable.
- **Deterministic simulation** — fixed 120 Hz updates, serializable RNG state, no wall-clock timers.
- **Reliable saves** — versioned JSON, strict validation, atomic replace, automatic `.bak` recovery.
- **AllMyPeople rework** — 16 seconds, 16 friendly units per second, persistently advancing and fighting.
- **No startup splash** — the game opens directly on the main menu.
- **Unit and smoke tests** — GoogleTest suite plus a CI-rendered smoke run under Xvfb.

## Requirements

- CMake **3.24+**
- A C++20 compiler (GCC 13+, Clang 16+, or MSVC 2022)
- **SFML 3.0+**
- `nlohmann_json` (header-only; CMake fetches a pinned release if it is not installed)
- GoogleTest (test-only; CMake fetches a pinned release if it is not installed)

SFML 2 and Boost are **not** supported.

### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake ninja sfml nlohmann-json gtest
```

### Ubuntu 26.04

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build libsfml-dev nlohmann-json3-dev libgtest-dev
```

Ubuntu 26.04 ships SFML 3.0.x. The code targets the stable SFML 3 API; it was also built against
SFML 3.1.0 locally.

## Building and Testing

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/thunder_wings
```

Useful CMake options:

| Option | Default | Meaning |
| --- | --- | --- |
| `TW_BUILD_TESTS` | `ON` | Build the GoogleTest suite |
| `TW_ENABLE_SANITIZERS` | `OFF` | Enable ASan/UBSan with GCC/Clang |

Debug build with sanitizers:

```bash
cmake -S . -B build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DTW_ENABLE_SANITIZERS=ON
cmake --build build-debug --parallel
ctest --test-dir build-debug --output-on-failure
```

Headless CI smoke check:

```bash
xvfb-run -a ./build/thunder_wings --smoke-test --no-audio
```

### Windows

1. Install Visual Studio 2022, CMake and Ninja.
2. Download the official **SFML 3.1.0 Windows VS2022 x64** archive:
   <https://github.com/SFML/SFML/releases/download/3.1.0/SFML-3.1.0-windows-vc17-64-bit.zip>
3. Extract it (for example to `C:\SFML`) and configure:

```powershell
cmake -S . -B build -A x64 `
  -DSFML_DIR="C:\SFML\lib\cmake\SFML" `
  -DTW_BUILD_TESTS=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

CMake copies the SFML DLLs and the assets next to `thunder_wings.exe`. nlohmann/json and
GoogleTest are fetched by CMake when they are not already present.

## Running and Controls

```text
Arrow keys   Move
P            Pause / resume
Esc          Return to menu (or back from Guide/About)
Up / Down    Select an option
PageUp/Down  Jump to first/last option
Enter        Confirm
```

The menu starts directly; there is no logo splash screen.

Command-line options:

```text
-h, --help             Show help
-v, --version          Show version/build information
    --no-audio         Disable music and sound
    --smoke-test       Load assets, render a deterministic 2 s game and exit
    --assets-dir PATH  Override asset directory
    --save-file PATH   Override the save file location
```

## Save System

Save from the pause menu (`P` → **Save Progress**). The default locations are:

- Linux: `$XDG_CONFIG_HOME/thunder-wings/game_data.json`, or
  `$HOME/.config/thunder-wings/game_data.json`
- Windows: `%APPDATA%\ThunderWings\game_data.json`

The format is versioned (`version: 2`). It includes the player, enemies, bullets, gifts, summoned
allies, timers and RNG state so a resumed game continues deterministically. Writes go to a temporary
file and are atomically renamed; the previous good file is kept as `game_data.json.bak` and used
automatically if the primary file is corrupt. Invalid, truncated, oversized or incompatible saves
are rejected with the same user-visible error instead of crashing; no version-specific migration or
detection is attempted.

## Gameplay Notes

### AllMyPeople

The old “charm on bullet hit” behaviour was replaced with a timed summoning effect:

- lasts **16 seconds**;
- produces **16 allies per second** (one wave of 16 immediately, then once per second, 16 waves,
  256 allies total);
- allies spawn just below the screen, advance upward at **90 px/s** (they never stop at the bottom
  edge) and leave the screen at the top;
- each ally has 2048 HP, targets the nearest enemy and fires a 512-damage bullet every 0.8 s;
- enemy bullets can destroy allies; player/ally bullets cannot hurt them;
- kills by allies still credit the player and trigger the normal kill heal;
- existing allies remain after the 16-second effect ends.

### Balance

All original player/enemy/bullet/gift numbers, spawn probabilities, boss patterns, artwork, audio,
fonts and HUD layout are preserved. The remaining gift weights are
`FullFirePower: 20`, `CenturyShield: 25`, `AllMyPeople: 15`.

## Architecture

```text
thunder_core (no SFML)
├── core/          Vec2/AABB, serializable RNG, logging
├── game/          GameWorld, entities, tuning, game state
└── persistence/   SaveData, versioned codec, atomic repository

thunder_wings (SFML 3)
├── assets/        catalog + instance-owned resources
├── render/        game renderer, menu renderer
├── app/           application state machine, CLI
└── main.cpp
```

`GameWorld` is a pure fixed-step simulation: it takes `PlayerInput`, emits sound IDs and can be
snapshotted. `GameRenderer` only reads the world. `AssetManager` owns every SFML resource; there are
no static texture/font/audio registries. See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for details.

## Tests

The GoogleTest suite covers vector/AABB math, deterministic RNG, player movement/shooting/regen,
damage reduction, enemy kills, AllMyPeople wave timing and combat, save round-trips and deterministic
resumption, malformed/incompatible saves, backup recovery, atomic writes and asset-catalog integrity.

```bash
ctest --test-dir build --output-on-failure
```

CI builds and tests Linux (SFML 3.0.x) and Windows (SFML 3.1.0), and runs the headless smoke test
under Xvfb.

## License

Copyright 2025 Nuo Shen, Nanjing University

Licensed under the Apache License, Version 2.0. You may obtain a copy at
<http://www.apache.org/licenses/LICENSE-2.0>.

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See [LICENSE](LICENSE) for details.
