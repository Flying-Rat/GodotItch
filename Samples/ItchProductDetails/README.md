# GodotItch Sample - Purchase Verification

This sample demonstrates how to use the GodotItch plugin for itch.io purchase verification in Godot games.

## What's Changed

Based on the code review, the following improvements have been made:

### ✅ Removed Metadata Functionality
- **Removed**: `itch_api.gd` - contained Open Graph metadata reading not needed for purchase verification
- **Updated**: Plugin description to focus on purchase verification
- **Cleaned**: All metadata-related code and dependencies

### ✅ Improved Plugin Access
- **Before**: `get_node_or_null("/root/Itch")` - required knowing autoload structure
- **After**: `GodotItch.verify()` - clean static API interface
- **Benefits**: No autoload dependency in user code, cleaner API

### ✅ Clear Class Purposes

- **`GodotItch`** (godot_itch.gd): Clean static API interface for developers
- **`Itch`** (itch.gd): Main autoload managing verification workflow  
- **`ItchVerificationClient`**: HTTP client for itch.io API communication
- **Supporting classes**: Utilities for validation, configuration, and data structures

### ✅ Enhanced Documentation
- **Created**: Comprehensive README with architecture overview
- **Added**: API reference and usage examples
- **Included**: Troubleshooting guide and security considerations

## Quick Start

1. **Enable the plugin** in Project Settings > Plugins
2. **Configure settings** in Project Settings:
   - `godot_itch/api_key`: Your itch.io API key
   - `godot_itch/game_id`: Your game's numeric ID
3. **Use the clean API**:

```gdscript
# Connect to signals
GodotItch.connect_verification_completed(_on_verified)
GodotItch.connect_verification_failed(_on_failed)

# Verify a download key
GodotItch.verify("user_download_key_or_url")
```

## Examples

- **`purchase_verification_example.gd`**: Complete UI example with validation and error handling
- **`simple_usage_example.gd`**: Minimal implementation showing the clean API

## Plugin Architecture

```
addons/godot_itch/
├── godot_itch.gd           # 🎯 Main API interface (your entry point)
├── plugin.cfg              # Plugin metadata  
├── plugin.gd               # Plugin registration
├── autoload/itch.gd        # Main verification workflow
├── core/                   # Utilities and configuration
└── verification/           # API client and result handling
```

The plugin now provides a focused, clean interface specifically for purchase verification without unnecessary metadata features.