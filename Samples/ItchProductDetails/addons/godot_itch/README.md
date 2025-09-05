# GodotItch Plugin

A Godot plugin for itch.io purchase verification. Verify that players have legitimately purchased your game by validating their download keys against the itch.io API.

## Features

- ✅ Download key validation and verification
- ✅ Support for multiple input formats (URLs, raw keys)
- ✅ Async API with signals for non-blocking verification
- ✅ Clean, simple API interface
- ✅ Automatic project settings integration
- ✅ Comprehensive error handling and validation
- ✅ Debug logging support

## Quick Start

### 1. Installation

1. Copy the `addons/godot_itch` folder to your project's `addons/` directory
2. Enable the plugin in Project Settings > Plugins
3. Configure your itch.io credentials in Project Settings

### 2. Configuration

Set these values in Project Settings:

- **`godot_itch/api_key`**: Your itch.io API key (from your [itch.io account settings](https://itch.io/user/settings/api-keys))
- **`godot_itch/game_id`**: Your game's numeric ID from itch.io
- **`godot_itch/require_verification`**: Whether verification is mandatory (default: true)
- **`godot_itch/debug_logging`**: Enable debug output (default: false)

### 3. Basic Usage

```gdscript
extends Control

func _ready():
    # Connect to verification signals
    GodotItch.connect_verification_completed(_on_verification_completed)
    GodotItch.connect_verification_failed(_on_verification_failed)
    
    # Verify a download key
    GodotItch.verify("user_download_key_here")

func _on_verification_completed(user_info: Dictionary):
    print("Verification successful!")
    print("User: ", user_info.display_name)
    print("User ID: ", user_info.user_id)
    # Grant access to premium content

func _on_verification_failed(error_message: String, error_code: String):
    print("Verification failed: ", error_message)
    # Handle verification failure
```

## API Reference

### GodotItch Class (Main Interface)

The primary interface for interacting with the plugin.

#### Static Methods

- **`verify(download_input: String)`** - Verify a download key or URL
- **`validate(download_input: String) -> Dictionary`** - Validate input without network request
- **`get_verification_status() -> Dictionary`** - Get current verification state
- **`clear_verification()`** - Clear verification status
- **`is_verification_required() -> bool`** - Check if verification is mandatory
- **`get_plugin_info() -> Dictionary`** - Get plugin status information

#### Static Signals

- **`verification_started()`** - Emitted when verification begins
- **`verification_completed(user_info: Dictionary)`** - Emitted on successful verification
- **`verification_failed(error_message: String, error_code: String)`** - Emitted on failure

### Input Formats

The plugin accepts multiple input formats:

```gdscript
# Raw download key
GodotItch.verify("abc123def456...")

# Full itch.io download URL
GodotItch.verify("https://username.itch.io/game/download/12345?key=abc123def456...")

# Download URL with key parameter
GodotItch.verify("https://itch.io/game-download/?key=abc123def456...")
```

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

## Plugin Architecture

### Core Classes

#### `GodotItch` (godot_itch.gd)
- **Purpose**: Clean static API interface for easy plugin access
- **Usage**: Primary interface for developers
- **Features**: Static methods and signal access, no autoload dependency in user code

#### `Itch` (autoload/itch.gd) 
- **Purpose**: Main autoload singleton managing verification workflow
- **Features**: Signal management, input validation, verification orchestration
- **Internal**: Automatically registered as autoload when plugin is enabled

#### `ItchVerificationClient` (verification/verification_client.gd)
- **Purpose**: HTTP client for itch.io API communication
- **Features**: API request handling, response parsing, error management
- **Extends**: HTTPRequest for network functionality

#### `ItchVerificationResult` (verification/verification_result.gd)
- **Purpose**: Data structure for verification results
- **Features**: User info storage, success/failure state, serialization support
- **Extends**: Resource for easy data handling

#### `ItchDownloadKey` (core/download_key.gd)
- **Purpose**: Download key validation and URL parsing utilities
- **Features**: Format validation, URL extraction, error reporting
- **Type**: Static utility class

#### `ItchInputProcessor` (core/input_processor.gd)
- **Purpose**: Input processing and validation logic
- **Features**: Multi-format input handling, validation results
- **Type**: Static utility class

#### `GodotItchConfig` (core/config.gd)
- **Purpose**: Project settings management and validation
- **Features**: Setting access, configuration validation
- **Type**: Static configuration helper

### Plugin Structure

```
addons/godot_itch/
├── godot_itch.gd           # Main API interface
├── plugin.cfg              # Plugin configuration
├── plugin.gd               # Plugin registration and setup
├── autoload/
│   └── itch.gd             # Main autoload singleton
├── core/
│   ├── config.gd           # Configuration helper
│   ├── download_key.gd     # Key validation utilities
│   └── input_processor.gd  # Input processing logic
└── verification/
    ├── verification_client.gd  # API client
    └── verification_result.gd  # Result data structure
```

## Error Handling

The plugin provides detailed error information:

- **`INVALID_INPUT`** - Invalid download key format or URL
- **`CONFIG_ERROR`** - Missing or invalid API configuration
- **`API_ERROR`** - Network or itch.io API errors

## Security Considerations

- API keys are stored in project settings (use environment variables in production)
- Download keys are validated client-side before API calls
- Network requests use HTTPS for secure communication
- No sensitive data is logged unless debug mode is enabled

## Troubleshooting

### Common Issues

1. **"Plugin not enabled" or "Failed to retrieve non-existent singleton 'Itch'"**
   - Enable the plugin in Project Settings > Plugins
   - **Important**: Restart Godot after enabling the plugin for the autoload to be properly initialized
   - Check that the autoload appears in Project Settings > AutoLoad

2. **"API key not set"**
   - Set your itch.io API key in Project Settings > godot_itch/api_key

3. **"Game ID not set"**
   - Set your game's numeric ID in Project Settings > godot_itch/game_id

4. **"Invalid download key format"**
   - Ensure the key is 20-64 characters, alphanumeric with underscores

5. **Autoload not found after enabling plugin**
   - Disable the plugin, restart Godot, then re-enable the plugin
   - Check Project Settings > AutoLoad for "Itch" entry
   - Verify the path points to `res://addons/godot_itch/autoload/itch.gd`

### Debug Mode

Enable debug logging in Project Settings:
```
godot_itch/debug_logging = true
```

This will output detailed verification steps to the console.

## Examples

See `examples/purchase_verification_example.gd` for a complete implementation example with UI.

## License

This plugin is provided as-is for itch.io game developers. See the main project license for details.

## Support

For issues and support, please refer to the main GodotItch repository.
