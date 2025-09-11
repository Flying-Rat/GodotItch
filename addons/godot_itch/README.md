# GodotItch Plugin

A Godot plugin for itch.io purchase verification. Verify that players have legitimately purchased your game by validating their download keys against the itch.io API.

## 🎮 Simple Demo

Try the basic demo to test the plugin:
- **Direct Access**: Load `res://addons/godot_itch/godot_itch_showcase.tscn`
- **Features**: Simple 3-field form with basic verification testing

The demo provides a minimal example showing the essential plugin functionality.

## Features

- Purchase verification using download keys or URLs
- Simple API for integration
- Debug logging support
- Project settings integration

## Table of Contents

- [Installation](#installation)
- [Configuration](#configuration)
- [Quick Start](#quick-start)
- [Testing](#testing)
- [Debug Mode](#debug-mode)
- [Error Codes](#error-codes)
- [License](#license)

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

## Quick Start

```gdscript
func _ready() -> void:
    Itch.initialize_with_scene(self)
    Itch.verify_purchase_result.connect(_on_verify_purchase_result)

func _on_verify_purchase_result(verified: bool, data: Dictionary) -> void:
    if verified:
        print("Verification succeeded:", data)
    else:
        print("Verification failed:", data)

# To verify a purchase
func verify_purchase(download_key: String) -> void:
    # Set required project settings if not already set
    ProjectSettings.set_setting("godot_itch/api_key", "your_api_key")
    ProjectSettings.set_setting("godot_itch/game_id", "your_game_id")
    
    Itch.verify_purchase(download_key)
```

## Testing
Run the included test suite:
```bash
godot --headless tests/verify_download_key_test.tscn
```

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

## License

MIT — see the repository `LICENSE` file for details.

---

**Updated based on tests**: Using `Itch` class is recommended; `call_deferred()` is unnecessary for connecting/calling from `_ready()`.
