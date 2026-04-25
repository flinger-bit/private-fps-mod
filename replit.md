# Private FPS Pro — Geode Mod

## Overview
Native C++ mod for Geometry Dash on Android, built with the Geode SDK (5.5.3, GD 2.2081). Compiles to a `.geode` package and is sideloaded into the game on a phone. **Not** a web application.

## Project Structure
- `src/main.cpp` — entry / PauseLayer hook adding the in-game hub button.
- `src/bot/` — `BotManager`, `Macro`, `FrameStepper` (recorder/player + stepping).
- `src/ui/` — `HubPopup` and the FPS / Bot tabs.
- `resources/` — sprites (`icon.png`).
- `mod.json` — Geode manifest, settings (FPS target, bot, frame stepper, CBF boost, swift clicks).
- `CMakeLists.txt` — CMake build, requires `GEODE_SDK` env var.
- `.github/workflows/build.yml` — CI build via `geode-sdk/build-geode-mod`, produces `.geode` artifact.

## Replit Setup
The project cannot truly "run" in Replit because it needs the Geode SDK + Android NDK toolchain plus the GD app on a phone. To still satisfy Replit's preview, a static info page is served:

- `web/index.html` — info page describing the mod, features, layout, and build instructions.
- Workflow `Start application` runs `python3 -m http.server 5000 --bind 0.0.0.0 --directory web` on port 5000 (webview).

## Building the Mod (locally / CI)
```
export GEODE_SDK=/path/to/geode-sdk
geode sdk install-binaries
geode build -p android64
```
Or push to `main` / dispatch the GitHub Actions workflow to get a `.geode` artifact.

## Deployment
Configured as Autoscale, serving the static info page from `web/`.
