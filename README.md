# Thunder Wings

**Thunder Wings** is a classic vertical-scrolling shooter game developed using C++ and the SFML library.

* ***built for NJU Advanced Program Design***

## Continuous Integration Status

| Status (main) | Description |
| :-----------: | :---------: |
| [![build status](https://github.com/snnbyyds/Thunder_Wings/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/snnbyyds/Thunder_Wings/actions/workflows/build-package.yml?query=branch%3Amain) | Build and Package |


## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Building from Source](#building-from-source)
  - [Linux (Debian)](#linux-debian)
  - [Windows](#windows)
- [Running the Game](#running-the-game)
- [Controls](#controls)
- [License](#license)

## Features

- **Classic Gameplay**: Vertical-scrolling shooter with waves of enemies and bosses.
- **Boss Battles**: Unique and challenging boss encounters.
- **Smooth Animations**: Fluid visuals for ships, enemies, and explosions.

## Prerequisites

Before building the project, ensure you have the following installed:

- **CMake** (version 3.10 or higher)
- **SFML** (version 2.5 ~ 2.6)
- **C++20** compatible compiler (e.g., GCC, MSVC)

### Linux (Debian)

Install dependencies:

```bash
sudo apt update
sudo apt install libsfml-dev cmake g++
```

### Windows

- Install [CMake](https://cmake.org/download/).
- Install [SFML](https://www.sfml-dev.org/download.php).
- Install a `C++` compiler (e.g., MSVC via Visual Studio).

## Building from Source

### Linux (Debian)

1. **Clone the repository**:

   ```bash
   git clone https://github.com/snnbyyds/Thunder_Wings.git
   cd Thunder_Wings
   ```

2. **Build the project**:

   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --config Release
   ```

### Windows

1. **Clone the repository**:

   ```bash
   git clone https://github.com/snnbyyds/Thunder_Wings.git
   cd Thunder_Wings
   ```

2. **Install SFML**:

   - [Download SFML 2.6.2](https://www.sfml-dev.org/download/sfml/2.6.2/)


3. **Build the project**:

   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DSFML_DIR="[full-path-to-sfml]/lib/cmake/SFML"
   cmake --build . --config Release
   ```

## Controls

- **Arrow Keys**: Move the spaceship (left, right, up, down).

## License

Copyright 2025 Nuo Shen, Nanjing University

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the [LICENSE](LICENSE) file for the specific language governing permissions and limitations under the License.