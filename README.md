# SkillTreeApp (Qt 6, C++17)

A cross-platform desktop skill tree editor/viewer built with Qt Widgets and clean architecture boundaries.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/SkillTreeApp
```

On Windows use:

```powershell
.\\build\\SkillTreeApp.exe
```

## Features

- Hierarchical skill tree with icon + name rendering
- Normal mode with tooltip + persistent side detail panel
- Editor mode toggle in `Settings -> Editor Mode`
- Right-click context actions: edit, delete, add child
- Reusable side editor panel for create/edit
- Dice expressions clickable inside long description (`1d20`, `2d6+1`, `1d20+1d4+3`)
- Roll results shown in always-visible bottom-left panel with hover breakdown
- Inline help via bracketed terms (`[Strength]`) with click popups
- JSON persistence for tree, custom fields, and help entries

## Data File

The app stores data in Qt AppData location as `skill_tree.json`.
