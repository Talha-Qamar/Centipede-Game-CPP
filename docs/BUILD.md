# Build Guide

## Prerequisites

- Windows with Visual Studio (2019/2022)
- SFML libraries configured for the project

## Option 1: Visual Studio (recommended)

1. Open the solution that contains this project.
2. Select build configuration (`Debug` or `Release`) and target platform (`x64`).
3. Build the project.
4. Run the generated executable from the project output directory.

## Option 2: Other Environments

This repository is currently configured primarily for Visual Studio (`Centipede.vcxproj`).
If you want cross-platform command-line builds, add a `CMakeLists.txt` and link SFML there.

## Runtime Working Directory

Make sure the executable runs with this repository root as working directory so these paths resolve:

- `Textures/...`
- `Music/...`
- `fonts/...`

## Troubleshooting

- If textures/audio fail to load, verify asset folders are present and path case matches exactly.
- If linker errors occur, re-check SFML include/lib paths and runtime DLL availability.
