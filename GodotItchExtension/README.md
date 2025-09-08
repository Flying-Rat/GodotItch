# GodotItchExtension

This folder contains the Godot GDExtension for integrating with the itch.io API, including build scripts, demo scenes, and source code.

## Features
- C++ GDExtension for Godot 4.4
- itch.io API integration (API key, download keys, etc.)
- Demo Godot project for testing and development
- PowerShell setup script for easy build

## Folder Structure
- `src/` — C++ source code for the extension
- `godot-cpp/` — Godot C++ bindings (submodule)
- `demo/` — Godot project with test scenes and scripts
- `bin/` — Compiled binaries and extension files
- `setup_gdextension.ps1` — PowerShell script for initial build
- `extension_api.json` — Custom API file for Godot bindings

## Initial Setup
1. Install [scons](https://scons.org/) and ensure it is in your PATH.
2. Open PowerShell in this folder.
3. Run the setup script:
   ```powershell
   ./setup_gdextension.ps1
   ```
   This will build `godot-cpp` and the GDExtension.

## Demo Project
- The `demo/` folder contains a Godot project for testing the extension.
- Open `demo/project.godot` in Godot 4.x.
- Use the UI to interact with itch.io API endpoints.

## Development
- Edit C++ source in `src/`.
- Rebuild using the setup script or manually with scons.
- For debugging, see `.vscode/` for example launch configurations.

## License
See [LICENSE](../LICENSE) in the root of the repository.
