# Itch Plugin

A Godot plugin for itch.io purchase verification. Verify that players have legitimately purchased your game by validating their download keys against the itch.io API.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Configuration](#configuration)
- [Quick Start](#quick-start)
# GodotItch

Itch.io purchase verification plugin for Godot.

## Installation

1. Copy the `addons/godot_itch` folder into your project's `addons/` directory.
2. Enable the plugin in Project Settings → Plugins.
3. Restart the editor.

## Configuration

Required project settings:

- `godot_itch/api_key` — Your itch.io API key
- `godot_itch/game_id` — Your game ID on itch.io

Important: You must set both `godot_itch/api_key` and `godot_itch/game_id` in Project Settings (Project → Project Settings → Itch) before using the plugin. The plugin will not function without them.

Optional:

- `godot_itch/debug_logging` — Enable debug output (default: `false`)

## Quick usage

```gdscript
Itch.verification_completed.connect(_on_verified)
Itch.verification_failed.connect(_on_failed)
Itch.verify("user_download_key_or_url")

func _on_verified(user_info: Dictionary):
    print("Verified:", user_info)

func _on_failed(err, code):
    print("Verification failed:", err)
```

## License

MIT — see the repository `LICENSE` file for details.

   - Check autoload path: `res://addons/godot_itch/autoload/itch.gd`

### Debug Mode

Enable debug logging:
```
godot_itch/debug_logging = true
```

This outputs detailed verification steps to the console.

### Error Codes

- **`INVALID_INPUT`** - Invalid key format or URL
- **`CONFIG_ERROR`** - Missing API configuration
- **`API_ERROR`** - Network or itch.io API errors

## Contributing

- Code organized into `autoload/`, `core/`, and `verification/`
-- Update `Itch._get_itch_singleton()` if autoload registration changes
- See main project for contribution guidelines

## License

This plugin is provided as-is for itch.io game developers. See the main project license for details.

---

**Updated based on tests**: Using `Itch` class is recommended; `call_deferred()` is unnecessary for connecting/calling from `_ready()`.
