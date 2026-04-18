# Centipede (C++ / SFML)

A desktop arcade game inspired by the classic Centipede, built in C++ using SFML.

## Highlights

- Real-time gameplay loop with menu, play, and game-over states
- Sprite-based rendering and audio playback via SFML
- Grid-based mushroom field and segmented centipede logic
- Score tracking and win/loss conditions

## Tech Stack

- C++
- SFML (Graphics + Audio)
- Visual Studio project files (`.vcxproj`) included

## Project Structure

See [docs/PROJECT_STRUCTURE.md](docs/PROJECT_STRUCTURE.md) for a full folder breakdown.

## Build and Run

See [docs/BUILD.md](docs/BUILD.md) for setup steps and run instructions.

## Assets

Runtime assets used by the game:

- `Textures/background.png`
- `Textures/player.png`
- `Textures/bullet.png`
- `Textures/centipede.png`
- `Textures/head.png`
- `Textures/mushroom.png`
- `Music/field_of_hopes.ogg`
- `fonts/arial.ttf`

## Notes

- Build artifacts are excluded via `.gitignore`.
- The repository keeps source and asset files needed to run the game.
