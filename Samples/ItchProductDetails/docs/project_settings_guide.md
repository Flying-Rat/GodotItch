# GodotItch Project Settings Configuration

## Overview

The GodotItch plugin now supports project settings configuration, making it easy to set up your API credentials and verification options without hardcoding them in your scripts.

## Available Settings

After enabling the plugin, you'll find these settings in **Project > Project Settings > General** under the `godot_itch/` section:

### `godot_itch/api_key` (String)
- **Description**: Your itch.io API key for server-side verification
- **Type**: Password field (hidden input)
- **Required**: Yes, for verification to work
- **How to get**: Visit https://itch.io/user/settings/api-keys and generate a new key

### `godot_itch/game_id` (String)
- **Description**: Your game's numeric ID from itch.io
- **Type**: Text field with placeholder
- **Required**: Yes, for verification to work
- **How to get**: Check your game's URL or itch.io dashboard

### `godot_itch/require_verification` (Boolean)
- **Description**: Whether download key verification is mandatory
- **Type**: Checkbox
- **Default**: `true`
- **Usage**: Set to `false` to disable verification (for testing, demos, etc.)

### `godot_itch/cache_duration_days` (Integer)
- **Description**: How many days to cache successful verification results
- **Type**: Number slider (1-30)
- **Default**: `7`
- **Usage**: Longer duration = less API calls, but less fresh verification

## Configuration Steps

### 1. Enable the Plugin
1. Go to **Project > Project Settings > Plugins**
2. Find "GodotItch" and enable it
3. The plugin will automatically add the project settings

### 2. Configure Your Settings
1. Go to **Project > Project Settings > General**
2. Scroll down to find the `godot_itch/` section
3. Fill in your values:
   - **api_key**: Your itch.io API key
   - **game_id**: Your game's numeric ID
   - **require_verification**: Keep `true` for production
   - **cache_duration_days**: Adjust as needed (7 days is recommended)

### 3. Save and Test
1. Click "Close" to save the settings
2. Run your verification test scene to confirm it works

## Usage in Code

### Using the Configuration Helper

```gdscript
# Check if plugin is configured
if GodotItchConfig.is_configured():
    print("Plugin is ready!")
else:
    print("Please configure API key and game ID")

# Get individual settings
var api_key = GodotItchConfig.get_api_key()
var game_id = GodotItchConfig.get_game_id()
var require_verification = GodotItchConfig.get_require_verification()

# Validate configuration
var errors = GodotItchConfig.validate_config()
if not errors.is_empty():
    for error in errors:
        print("Config error: " + error)
```

### Using the Verification Manager (Recommended)

```gdscript
# In your main game scene
func _ready():
    if GodotItchManager.needs_verification():
        show_verification_ui()
    else:
        start_game()

func show_verification_ui():
    # Show your verification UI
    # When user enters download key:
    GodotItchManager.verify_download_key(user_download_key)

func _on_verification_completed(result: ItchVerificationResult):
    print("User verified: " + result.get_user_display_name())
    start_game()

func _on_verification_failed(error: String):
    print("Verification failed: " + error)
    # Show error to user
```

### Direct Verification Client Usage

```gdscript
# For advanced use cases
var client = ItchVerificationClient.new()
add_child(client)

client.verification_completed.connect(_on_verification_completed)
client.verification_failed.connect(_on_verification_failed)

# Use project settings automatically
client.verify_download_key(
    download_key,
    GodotItchConfig.get_api_key(),
    GodotItchConfig.get_game_id()
)
```

## Security Notes

### API Key Protection
- Your API key is stored in the project settings file (`project.godot`)
- **Important**: For production builds, consider additional obfuscation
- The API key field is marked as password type (hidden in editor)
- Never commit your API key to public repositories

### Recommended Practices
1. Use different API keys for development and production
2. Regularly rotate your API keys
3. Monitor API key usage in your itch.io dashboard
4. Set appropriate cache duration to balance security and performance

## Troubleshooting

### "Plugin not configured" Error
- Check that both `api_key` and `game_id` are set in project settings
- Verify your API key is valid by testing it manually
- Ensure game ID is numeric and matches your itch.io game

### Verification Still Uses Hardcoded Values
- Make sure you've restarted Godot after configuring settings
- Check that your code is using `GodotItchConfig` or `GodotItchManager`
- Verify the plugin is enabled in Project Settings > Plugins

### Settings Don't Appear
- Ensure the plugin is enabled
- Try disabling and re-enabling the plugin
- Check for any console errors when enabling the plugin

## Example Configuration

For the test project, you would configure:

```
godot_itch/api_key = "5F2BrjiAXZAD4RYYRzd0tVvxU0aFvDps4OJ38yum"
godot_itch/game_id = "3719972"
godot_itch/require_verification = true
godot_itch/cache_duration_days = 7
```

After configuration, the verification test will automatically use these values instead of the hardcoded fallbacks.
