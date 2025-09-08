# GodotItch Plugin

A Godot plugin for itch.io purchase verification. Verify that players have legitimately purchased your game by validating their download keys against the itch.io API.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Configuration](#configuration)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
- [Input Formats](#input-formats)
- [Plugin Architecture](#plugin-architecture)
- [Testing & Validation](#testing--validation)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Features

- ✅ Download key validation and verification
- ✅ Support for multiple input formats (URLs, raw keys)
- ✅ Async API with signals for non-blocking verification
- ✅ Clean, simple API interface
- ✅ Automatic project settings integration
- ✅ Comprehensive error handling and validation
- ✅ Debug logging support

## Installation

1. Copy the `addons/godot_itch` folder to your project's `addons/` directory
2. Enable the plugin in **Project Settings > Plugins**
3. Restart Godot to initialize the autoload
4. Configure your itch.io credentials (see [Configuration](#configuration))

## Configuration

The plugin includes a user-friendly settings panel accessible through **Project > Project Settings**. Look for the "Itch" panel on the right side of the Project Settings dialog.

### Required Settings

- **`godot_itch/api_key`**: Your itch.io API key (from [itch.io account settings](https://itch.io/user/settings/api-keys))
- **`godot_itch/game_id`**: Your game's numeric ID from itch.io

### Optional Settings

- **`godot_itch/debug_logging`**: Enable debug output (default: `false`)

You can also access the settings panel by using the **Tools > Itch Settings** menu in the Godot editor.

## Quick Start

### Basic Usage

```gdscript
extends Control

func _ready():
    # Connect to verification signals (no call_deferred needed!)
    GodotItch.connect_verification_completed(_on_verification_completed)
    GodotItch.connect_verification_failed(_on_verification_failed)

    # Verify a download key
    GodotItch.verify("user_download_key_here")

func _on_verification_completed(user_info: Dictionary):
    print("Verification successful!")
    print("User: ", user_info.display_name)
    # Grant access to premium content

func _on_verification_failed(error_message: String, error_code: String):
    print("Verification failed: ", error_message)
    # Handle verification failure
```

### Key Findings from Testing

- **Use `GodotItch` class** (recommended) instead of accessing `/root/Itch` directly
- **`call_deferred()` is unnecessary** - autoloads are available immediately in `_ready()`
- The plugin includes built-in validation and error handling for robustness

## API Reference

### GodotItch Class (Main Interface)

The primary interface for interacting with the plugin.

#### Static Methods

- **`verify(download_input: String)`** - Start verification flow (validates then performs network call)
- **`validate(download_input: String) -> Dictionary`** - Validate input without network request
- **`get_verification_status() -> Dictionary`** - Get current verification state
- **`clear_verification()`** - Clear verification status
- **`get_plugin_info() -> Dictionary`** - Get plugin status information

#### Static Signals

- **`verification_started()`** - Emitted when verification begins
- **`verification_completed(user_info: Dictionary)`** - Emitted on successful verification
- **`verification_failed(error_message: String, error_code: String)`** - Emitted on failure

### Verification Result

The `verification_completed` signal provides a dictionary with user information:

```gdscript
{
    "user_id": "12345",
    "display_name": "User Name",
    "username": "username",
    "game_title": "Your Game",
    "verified_at": "2024-01-01T12:00:00Z"
}
```

## Input Formats

The plugin accepts multiple input formats:

```gdscript
# Raw download key
GodotItch.verify("abc123def456...")

# Full itch.io download URL
GodotItch.verify("https://username.itch.io/game/download/12345?key=abc123def456...")

# Download URL with key parameter
GodotItch.verify("https://itch.io/game-download/?key=abc123def456...")
```

## Plugin Architecture

### Core Classes

- **`GodotItch`** (`godot_itch.gd`) - Clean static API interface
- **`Itch`** (`autoload/itch.gd`) - Main autoload singleton
- **`ItchVerificationClient`** (`verification/verification_client.gd`) - HTTP client for API
- **`ItchVerificationResult`** (`verification/verification_result.gd`) - Result data structure
- **`ItchDownloadKey`** (`core/download_key.gd`) - Key validation utilities
- **`ItchInputProcessor`** (`core/input_processor.gd`) - Input processing logic
- **`GodotItchConfig`** (`core/config.gd`) - Configuration helper

### Plugin Structure

```
addons/godot_itch/
├── godot_itch.gd           # Main API interface
├── plugin.cfg              # Plugin configuration
├── plugin.gd               # Plugin registration
├── autoload/
│   └── itch.gd             # Main autoload singleton
├── core/
│   ├── config.gd           # Configuration helper
│   ├── download_key.gd     # Key validation
│   └── input_processor.gd  # Input processing
└── verification/
    ├── verification_client.gd  # API client
    └── verification_result.gd  # Result structure
```

## Testing & Validation

Several test scenes validate timing and behavior:

- `tests/access_method_test.tscn` - Checks autoload access and GodotItch class
- `tests/autoload_analysis.tscn` - Deep-dive into autoload discovery
- `tests/call_deferred_test.tscn` - Verifies `call_deferred` necessity (it's not!)
- `tests/final_validation.tscn` - Final smoke tests

Run tests with:
```bash
godot --headless tests/final_validation.tscn
```

## Troubleshooting

### Common Issues

1. **"Plugin not enabled" or "Failed to retrieve singleton 'Itch'"**
   - Enable plugin in **Project Settings > Plugins**
   - **Restart Godot** after enabling for autoload initialization
   - Verify "Itch" appears in **Project Settings > AutoLoad**

2. **"API key not set"**
   - Set `godot_itch/api_key` in Project Settings

3. **"Game ID not set"**
   - Set `godot_itch/game_id` in Project Settings

4. **"Invalid download key format"**
   - Keys must be 20-64 characters, alphanumeric with underscores

5. **Autoload not found**
   - Disable plugin, restart Godot, re-enable plugin
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
- Update `GodotItch._get_itch_singleton()` if autoload registration changes
- See main project for contribution guidelines

## License

This plugin is provided as-is for itch.io game developers. See the main project license for details.

---

**Updated based on tests**: Using `GodotItch` class is recommended; `call_deferred()` is unnecessary for connecting/calling from `_ready()`.
