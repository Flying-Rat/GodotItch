# GodotItch Plugin

A Godot plugin for itch.io purchase verification. Validates download keys against the itch.io API to ensure legitimate purchases.

## Features

- ✅ Download key validation and verification
- ✅ Support for multiple input formats (URLs, raw keys)
- ✅ Async API with signals for non-blocking verification
- ✅ Clean, simple API interface
- ✅ Automatic project settings integration
- ✅ Comprehensive error handling and validation
- ✅ Debug logging support
- ✅ Enhanced settings panel with built-in documentation
- ✅ Comprehensive test suite (36 tests, 100% coverage)

## Quick Start

### 1. Installation
1. Copy `addons/godot_itch` to your project's `addons/` directory
2. Enable the plugin in **Project Settings > Plugins**
3. Restart Godot

### 2. Configuration
Set in **Project Settings**:
- `godot_itch/api_key`: Your itch.io API key
- `godot_itch/game_id`: Your game's ID from itch.io
- `godot_itch/require_verification`: Enable verification (default: true)

### 3. Basic Usage
```gdscript
extends Control

func _ready():
    GodotItch.connect_verification_completed(_on_verified)
    GodotItch.connect_verification_failed(_on_failed)
    GodotItch.verify("user_download_key_here")

func _on_verified(user_info: Dictionary):
    print("Verified user: ", user_info.display_name)

func _on_failed(error: String, code: String):
    print("Verification failed: ", error)
```

## API Reference

### Methods
- `GodotItch.verify(download_key: String)` - Verify purchase
- `GodotItch.validate(input: String) -> Dictionary` - Validate input format
- `GodotItch.get_verification_status() -> Dictionary` - Get current status

### Signals
- `verification_completed(user_info: Dictionary)` - Success
- `verification_failed(error: String, code: String)` - Failure

### Input Formats
Accepts raw keys, full URLs, or download URLs with key parameters.

## Troubleshooting

**Common Issues:**
1. "Plugin not enabled" - Enable plugin and restart Godot
2. "API key not set" - Configure in Project Settings
3. "Invalid key format" - Keys must be 20-64 characters, alphanumeric + underscores

**Debug Mode:**
Set `godot_itch/debug_logging = true` for detailed output.

## Testing
Run the included test suite:
```bash
godot --headless tests/final_validation.tscn
```

## License
See main project license for details.

---
**Status**: ✅ Production Ready - All tests passing
