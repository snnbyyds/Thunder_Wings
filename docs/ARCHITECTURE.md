# Architecture

## Module boundaries

| Module | Depends on | Responsibility |
| --- | --- | --- |
| `thunder::core` | C++ standard library | Vec2/AABB math, serializable RNG, logging |
| `thunder::game` | core | Deterministic gameplay, spawning, collisions, events, snapshots |
| `thunder::persistence` | game, core, nlohmann/json | Save schema, validation, atomic files, backup recovery |
| assets/render/app | core + SFML 3 | Resource lifetime, drawing, input/state machine |

The dependency direction is strict: the simulation never includes SFML and never knows about the
window, textures, fonts, audio or the filesystem. `GameWorld` accepts `PlayerInput` and produces
`SoundId` events plus world state. `Application` consumes those events and drives the renderer.

## Simulation

- Fixed time step: `1/120 s`, with frame-time catch-up capped at `0.25 s`.
- All timers are countdown floats advanced by `update(dt)`; there are no `sf::Clock` fields.
- Entities are plain value types stored in `std::vector`:
  `Player`, `Bullet`, `Enemy`, `Ally`, `Gift`, `Explosion`.
- Collision is AABB-based using the original sprite dimensions and positions
  (player position = centre; enemy position = top-left, matching the original renderer).
- Sound is emitted through `std::vector<SoundId>`; the core has no audio dependency.
- Transient full-screen explosion effects are visual-only and deliberately excluded from saves.

## Save schema (version 2)

Top level:

```json
{
  "format": "thunder-wings-save",
  "version": 2,
  "world": { }
}
```

`world` contains:

- `elapsed`, `kills`, `alliesSummoned`, `enemySpawnTimer`, `giftSpawnTimer`;
- `rng`: four unsigned 64-bit words;
- `player`: position, health, animation/death/cooldown state, shot counter and gifts;
- `enemies`, `bullets`, `allies`: full entity state.

Gifts preserve `kind`, `remaining`, `maxTime`, warning flags, AllMyPeople pulse budget and pulse
timer. Enemies preserve both pattern and animation state. Since the RNG is part of the snapshot,
resuming and updating both worlds produces byte-identical subsequent snapshots.

Validation rejects:

- missing/mismatched format or version;
- unknown enum strings;
- non-finite numbers;
- oversized arrays (bullets 8192, enemies 512, allies 1024, gifts 16);
- negative health/timers where the simulation requires non-negative values;
- malformed JSON and truncated files.

`SaveRepository` enforces a 16 MiB read limit, writes `game_data.json.tmp`, flushes and closes it,
keeps the previous file as `.bak`, and then renames the temp file into place. If the primary file
cannot be decoded, the `.bak` is tried.

## Rendering and resources

`AssetManager` is created after the window and owns all `sf::Texture`, `sf::Font`, `sf::SoundBuffer`,
`sf::Music` and voice-pool objects. The asset catalog maps stable enums to relative paths.
`MenuRenderer` draws the menu and Guide/About pages; `GameRenderer` draws the world, HUD, pause
overlay and game-over text. Neither renderer mutates the simulation.

## Application state machine

`Application` replaces the old recursive `Menu::show/start/load` flow with an explicit state:

```text
Menu -> Guide/About -> Menu
Menu -> Playing -> Paused -> Playing
             |          \-> Menu
             \-> GameOver -> Menu
```

Input is event-driven for menus and polled for player movement. Pausing freezes the simulation and
the music; resuming restarts both. Game over waits 3.2 seconds without blocking the event loop.
