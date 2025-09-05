# GodotItch Plugin

A Godot plugin for itch.io purchase verification, allowing developers to verify if users have legitimately purchased their game through itch.io.

## Features

- **Simple API**: Easy-to-use interface for purchase verification
- **Self-contained**: Plugin handles all itch.io API integration internally
- **Project Settings Integration**: Configure API keys and game ID through Godot's project settings
- **Real-time Verification**: Validates download keys against itch.io's API
- **Error Handling**: Comprehensive error reporting and user guidance

## Quick Start

### 1. Installation

1. Copy the `addons/godot_itch/` folder to your project
2. Enable "GodotItch" in Project Settings > Plugins

### 2. Configuration

Set these values in Project Settings > godot_itch:
- `api_key`: Your itch.io API key (get from your itch.io account settings)
- `game_id`: Your game's numeric ID from itch.io
- `require_verification`: Whether verification is required (default: true)

### 3. Basic Usage

```gdscript
extends Node

func _ready():
    # Get the plugin
    var itch = get_node("/root/Itch")
    
    # Connect to verification events
    itch.verification_completed.connect(_on_purchase_verified)
    itch.verification_failed.connect(_on_verification_failed)
    
    # Verify a user's download key
    verify_purchase("user_download_key_or_url")

func verify_purchase(download_key: String):
    var itch = get_node("/root/Itch")
    
    # Validate input
    var validation = itch.validate(download_key)
    if not validation.valid:
        print("Invalid key: " + validation.error)
        return
    
    # Start verification
    itch.verify(download_key)

func _on_purchase_verified(user_info: Dictionary):
    print("Purchase verified for: " + user_info.display_name)
    # Unlock premium content, save verification status, etc.

func _on_verification_failed(error: String, code: String):
    print("Verification failed: " + error)
    # Handle failure (show message, restrict features, etc.)
```

## API Reference

### Methods

- `validate(input: String) -> Dictionary`: Validate download key format
- `verify(download_key: String) -> void`: Start verification process
- `get_verification_status() -> Dictionary`: Get current verification state
- `clear_verification() -> void`: Clear cached verification
- `is_verification_required() -> bool`: Check if verification is required

### Signals

- `verification_started()`: Verification process has begun
- `verification_completed(user_info: Dictionary)`: Verification succeeded
- `verification_failed(error: String, code: String)`: Verification failed

## Example

See `examples/purchase_verification_example.tscn` for a complete implementation example with UI.

## License

This plugin is provided as-is for integration with itch.io services.