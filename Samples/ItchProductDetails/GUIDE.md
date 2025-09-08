# GodotItch Plugin - Developer Guide

A comprehensive guide to implementing and using the GodotItch plugin for itch.io purchase verification.

## Table of Contents

- [Getting Started](#getting-started)
- [Implementation Examples](#implementation-examples)  
- [Advanced Usage](#advanced-usage)
- [Configuration Details](#configuration-details)
- [Testing and Validation](#testing-and-validation)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

## Getting Started

### Prerequisites

- Godot 4.0+ (tested with 4.4.1)
- Active itch.io developer account
- Published game on itch.io with purchase verification enabled

### Step-by-Step Setup

#### 1. Plugin Installation

1. **Download/Copy Plugin**
   ```
   your_project/
   └── addons/
       └── godot_itch/
           ├── godot_itch.gd
           ├── plugin.cfg
           ├── plugin.gd
           └── ... (other plugin files)
   ```

2. **Enable in Godot**
   - Open **Project Settings > Plugins**
   - Find "GodotItch Purchase Verification" 
   - Enable the plugin
   - **Important**: Restart Godot after enabling

3. **Verify Installation**
   - Check **Project Settings > AutoLoad**
   - Confirm "Itch" autoload is registered
   - Path should be: `res://addons/godot_itch/autoload/itch.gd`

#### 2. Configuration

Open **Project Settings** and set these values:

```
Application/
└── godot_itch/
    ├── api_key: "your_itch_api_key_here"
    ├── game_id: 123456
    ├── require_verification: true
    ├── cache_duration_days: 7
    └── debug_logging: false
```

**Getting Your Credentials:**
- **API Key**: Go to [itch.io API Keys](https://itch.io/user/settings/api-keys)
- **Game ID**: Check your game's URL or itch.io dashboard

## Implementation Examples

### Basic Implementation

```gdscript
# Simple verification example
extends Control

func _ready():
    # Connect signals
    GodotItch.connect_verification_completed(_on_verification_success)
    GodotItch.connect_verification_failed(_on_verification_failed)

func verify_user_purchase(download_key: String):
    print("Starting verification...")
    GodotItch.verify(download_key)

func _on_verification_success(user_info: Dictionary):
    print("✅ Verification successful!")
    print("User: ", user_info.display_name)
    print("Game: ", user_info.game_title)
    
    # Enable premium features
    unlock_premium_content()

func _on_verification_failed(error_message: String, error_code: String):
    print("❌ Verification failed: ", error_message)
    print("Error code: ", error_code)
    
    # Handle failure
    show_verification_error(error_message)
```

### Complete UI Example

```gdscript
# Full UI implementation with input validation
extends Control

@onready var key_input: LineEdit = $VBoxContainer/KeyInput
@onready var verify_button: Button = $VBoxContainer/VerifyButton
@onready var status_label: Label = $VBoxContainer/StatusLabel
@onready var progress_bar: ProgressBar = $VBoxContainer/ProgressBar
@onready var result_panel: Panel = $VBoxContainer/ResultPanel

func _ready():
    # Setup UI
    verify_button.pressed.connect(_on_verify_pressed)
    key_input.text_changed.connect(_on_input_changed)
    
    # Connect verification signals
    GodotItch.connect_verification_started(_on_verification_started)
    GodotItch.connect_verification_completed(_on_verification_completed)
    GodotItch.connect_verification_failed(_on_verification_failed)
    
    # Initialize state
    update_ui_state()

func _on_input_changed(text: String):
    # Real-time validation feedback
    var validation = GodotItch.validate(text)
    
    if validation.valid:
        key_input.modulate = Color.WHITE
        verify_button.disabled = false
        status_label.text = "✅ Valid format"
    else:
        key_input.modulate = Color.LIGHT_SALMON
        verify_button.disabled = true
        status_label.text = "❌ " + validation.error_message

func _on_verify_pressed():
    var input_text = key_input.text.strip_edges()
    
    if input_text.is_empty():
        show_error("Please enter a download key or URL")
        return
    
    # Start verification
    GodotItch.verify(input_text)

func _on_verification_started():
    print("🔄 Verification started")
    verify_button.disabled = true
    progress_bar.visible = true
    status_label.text = "Verifying purchase..."

func _on_verification_completed(user_info: Dictionary):
    print("✅ Verification completed successfully")
    
    # Update UI
    verify_button.disabled = false
    progress_bar.visible = false
    result_panel.visible = true
    
    # Display success
    status_label.text = "✅ Verification successful!"
    show_user_info(user_info)
    
    # Unlock premium features
    enable_premium_content()

func _on_verification_failed(error_message: String, error_code: String):
    print("❌ Verification failed: ", error_message)
    
    # Update UI
    verify_button.disabled = false
    progress_bar.visible = false
    result_panel.visible = false
    
    # Show error
    status_label.text = "❌ " + error_message
    show_error_details(error_message, error_code)

func show_user_info(user_info: Dictionary):
    # Display user information
    var info_text = "Welcome, %s!\nGame: %s\nVerified: %s" % [
        user_info.display_name,
        user_info.game_title,
        user_info.verified_at
    ]
    # Update your UI labels here

func enable_premium_content():
    # Your game-specific premium unlock logic
    GameManager.unlock_premium_features()
    get_tree().change_scene_to_file("res://premium_main_menu.tscn")
```

### Game Integration Example

```gdscript
# Integration with your game's systems
extends Node

signal purchase_verified(user_data)
signal verification_failed(reason)

var is_premium_user: bool = false
var user_data: Dictionary = {}

func _ready():
    # Connect to plugin signals
    GodotItch.connect_verification_completed(_on_purchase_verified)
    GodotItch.connect_verification_failed(_on_purchase_failed)
    
    # Check if verification is required
    if GodotItch.is_verification_required():
        show_verification_screen()
    else:
        # Skip verification in development
        start_game_directly()

func verify_purchase(download_key: String):
    # Clear previous state
    is_premium_user = false
    user_data.clear()
    
    # Start verification
    GodotItch.verify(download_key)

func _on_purchase_verified(user_info: Dictionary):
    is_premium_user = true
    user_data = user_info
    
    # Store verification status
    save_verification_status(user_info)
    
    # Emit game signal
    purchase_verified.emit(user_info)
    
    # Continue to game
    start_premium_game()

func _on_purchase_failed(error_message: String, error_code: String):
    is_premium_user = false
    user_data.clear()
    
    # Log error for analytics
    log_verification_failure(error_code, error_message)
    
    # Emit game signal
    verification_failed.emit(error_message)
    
    # Handle based on error type
    match error_code:
        "INVALID_INPUT":
            show_input_error("Please check your download key format")
        "CONFIG_ERROR":
            show_system_error("Game configuration error")
        "API_ERROR":
            show_network_error("Network connection issue")
        _:
            show_generic_error(error_message)

func save_verification_status(user_info: Dictionary):
    # Save verification locally (encrypted)
    var save_data = {
        "verified": true,
        "user_id": user_info.user_id,
        "timestamp": Time.get_unix_time_from_system(),
        "game_version": ProjectSettings.get_setting("application/config/version")
    }
    
    var file = FileAccess.open("user://verification.dat", FileAccess.WRITE)
    if file:
        file.store_string(JSON.stringify(save_data))
        file.close()

func load_verification_status() -> bool:
    var file = FileAccess.open("user://verification.dat", FileAccess.READ)
    if not file:
        return false
    
    var json_string = file.get_as_text()
    file.close()
    
    var json = JSON.new()
    var parse_result = json.parse(json_string)
    
    if parse_result != OK:
        return false
    
    var data = json.get_data()
    
    # Check if verification is still valid (within cache duration)
    var cache_duration = GodotItchConfig.get_cache_duration_days() * 24 * 60 * 60
    var current_time = Time.get_unix_time_from_system()
    
    if current_time - data.get("timestamp", 0) < cache_duration:
        is_premium_user = data.get("verified", false)
        return is_premium_user
    
    return false
```

## Advanced Usage

### Custom Input Processing

```gdscript
# Handle different input formats
func process_user_input(raw_input: String) -> String:
    var cleaned = raw_input.strip_edges()
    
    # Handle different URL formats
    if cleaned.begins_with("https://"):
        # Extract key from URL
        var validation = GodotItch.validate(cleaned)
        if validation.valid:
            return validation.processed_key
    
    # Handle raw keys
    elif cleaned.length() >= 20:
        return cleaned
    
    # Handle clipboard content
    elif cleaned.is_empty():
        var clipboard = DisplayServer.clipboard_get()
        return process_user_input(clipboard)
    
    return cleaned
```

### Batch Verification

```gdscript
# Verify multiple keys (for bundle purchases)
func verify_multiple_keys(keys: Array[String]):
    for key in keys:
        # Add delay between requests to avoid rate limiting
        await get_tree().create_timer(1.0).timeout
        GodotItch.verify(key)
```

### Custom Error Handling

```gdscript
# Advanced error handling with retry logic
var verification_retries: int = 0
const MAX_RETRIES: int = 3

func _on_verification_failed(error_message: String, error_code: String):
    match error_code:
        "API_ERROR":
            if verification_retries < MAX_RETRIES:
                verification_retries += 1
                print("Retrying verification... (", verification_retries, "/", MAX_RETRIES, ")")
                
                # Wait before retry
                await get_tree().create_timer(2.0).timeout
                GodotItch.verify(last_key_attempted)
            else:
                show_max_retries_error()
        
        "INVALID_INPUT":
            show_format_helper()
        
        "CONFIG_ERROR":
            show_developer_contact()
```

## Configuration Details

### Project Settings Reference

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `godot_itch/api_key` | String | "" | Your itch.io API key |
| `godot_itch/game_id` | int | 0 | Your game's numeric ID |
| `godot_itch/require_verification` | bool | true | Whether verification is mandatory |
| `godot_itch/cache_duration_days` | int | 7 | Verification cache duration |
| `godot_itch/debug_logging` | bool | false | Enable debug output |

### Configuration Helper

```gdscript
# Access configuration programmatically
func check_plugin_configuration():
    var config = GodotItchConfig.new()
    
    print("API Key set: ", config.has_api_key())
    print("Game ID set: ", config.has_game_id())
    print("Verification required: ", config.is_verification_required())
    print("Cache duration: ", config.get_cache_duration_days(), " days")
    print("Debug enabled: ", config.is_debug_enabled())
```

## Testing and Validation

### Manual Testing Checklist

1. **Plugin Installation**
   - [ ] Plugin appears in Plugins list
   - [ ] Autoload registered correctly
   - [ ] Settings panel accessible

2. **Configuration**
   - [ ] API key accepts valid format
   - [ ] Game ID accepts numeric values
   - [ ] Test connection works

3. **Verification Flow**
   - [ ] Valid keys succeed
   - [ ] Invalid keys fail appropriately
   - [ ] Network errors handled gracefully
   - [ ] UI updates correctly

4. **Signal System**
   - [ ] verification_started emits
   - [ ] verification_completed with correct data
   - [ ] verification_failed with error details

### Automated Testing

```gdscript
# Run the included test suite
# In Godot: Scene > Change Scene > tests/final_validation.tscn
# Or command line: godot --headless tests/final_validation.tscn

# Check test results
func verify_test_suite():
    var test_scene = load("res://tests/final_validation.tscn")
    var test_instance = test_scene.instantiate()
    get_tree().root.add_child(test_instance)
```

## Best Practices

### Performance

1. **Cache Verification Results**
   ```gdscript
   # Don't verify on every game start
   if load_cached_verification():
       start_game_directly()
   else:
       request_verification()
   ```

2. **Async Operations**
   ```gdscript
   # Don't block the UI during verification
   func _on_verification_started():
       show_loading_spinner()
       disable_user_input()
   ```

3. **Error Recovery**
   ```gdscript
   # Provide fallback options
   func _on_verification_failed(error: String, code: String):
       if code == "API_ERROR":
           offer_offline_mode()
       else:
           show_manual_verification_option()
   ```

### Security

1. **Input Validation**
   ```gdscript
   # Always validate before verification
   var validation = GodotItch.validate(user_input)
   if not validation.valid:
       show_error(validation.error_message)
       return
   ```

2. **Secure Storage**
   ```gdscript
   # Encrypt sensitive data
   var file = FileAccess.open("user://verification.dat", FileAccess.WRITE)
   file.store_var(verification_data, true)  # true = encrypt
   ```

3. **Rate Limiting**
   ```gdscript
   # Prevent API abuse
   var last_verification_time: float = 0
   const MIN_VERIFICATION_INTERVAL: float = 5.0
   
   func verify_with_rate_limit(key: String):
       var current_time = Time.get_time_dict_from_system()
       if current_time - last_verification_time < MIN_VERIFICATION_INTERVAL:
           show_error("Please wait before trying again")
           return
       
       last_verification_time = current_time
       GodotItch.verify(key)
   ```

### User Experience

1. **Clear Feedback**
   ```gdscript
   # Provide helpful error messages
   func format_user_error(error_code: String) -> String:
       match error_code:
           "INVALID_INPUT":
               return "Please check your download key format. It should be 20-64 characters long."
           "API_ERROR":
               return "Connection issue. Please check your internet and try again."
           "CONFIG_ERROR":
               return "Game configuration error. Please contact support."
           _:
               return "Verification failed. Please try again or contact support."
   ```

2. **Progressive Enhancement**
   ```gdscript
   # Graceful degradation
   func handle_verification_requirement():
       if GodotItch.is_verification_required():
           if has_network_connection():
               show_verification_screen()
           else:
               offer_offline_trial_mode()
       else:
           start_game_directly()
   ```

## Troubleshooting

### Common Issues and Solutions

#### 1. "Plugin not enabled"
**Symptoms**: Error messages about missing autoload
**Solutions**:
- Enable plugin in Project Settings > Plugins
- Restart Godot after enabling
- Check AutoLoad settings for "Itch" entry

#### 2. "API key not set"
**Symptoms**: CONFIG_ERROR during verification
**Solutions**:
- Set `godot_itch/api_key` in Project Settings
- Verify API key is active on itch.io
- Check for extra spaces or formatting issues

#### 3. "Invalid download key format"
**Symptoms**: INVALID_INPUT error with valid-looking keys
**Solutions**:
- Keys must be 20-64 characters
- Only alphanumeric and underscores allowed
- Check for hidden characters or extra spaces

#### 4. Network/API errors
**Symptoms**: API_ERROR during verification
**Solutions**:
- Check internet connection
- Verify itch.io is accessible
- Check if your IP is rate-limited
- Try again in a few minutes

#### 5. Autoload registration issues
**Symptoms**: "Failed to retrieve singleton 'Itch'"
**Solutions**:
```gdscript
# Check autoload registration
func debug_autoload_status():
    var autoloads = ProjectSettings.get_setting("autoload")
    print("Registered autoloads: ", autoloads)
    
    # Try to access the autoload
    var itch_node = get_node_or_null("/root/Itch")
    if itch_node:
        print("✅ Itch autoload found")
    else:
        print("❌ Itch autoload not found")
```

### Debug Techniques

#### Enable Debug Logging
```gdscript
# In Project Settings
godot_itch/debug_logging = true

# Or programmatically
func enable_debug_mode():
    ProjectSettings.set_setting("godot_itch/debug_logging", true)
    ProjectSettings.save()
```

#### Test Connection
```gdscript
# Use the built-in test connection feature
func test_plugin_connectivity():
    # Available in the enhanced settings panel
    # Or programmatically check configuration
    
    var config = GodotItchConfig.new()
    if not config.has_api_key():
        print("❌ API key not set")
    elif not config.has_game_id():
        print("❌ Game ID not set")
    else:
        print("✅ Configuration appears valid")
        # Try a test verification with a dummy key
        GodotItch.validate("test_key_format_12345678901234567890")
```

#### Monitor Network Traffic
```gdscript
# Add logging to track API calls
func _on_verification_started():
    print("🌐 API Request started at: ", Time.get_datetime_string_from_system())

func _on_verification_completed(user_info: Dictionary):
    print("✅ API Response received at: ", Time.get_datetime_string_from_system())
    print("Response data: ", user_info)

func _on_verification_failed(error: String, code: String):
    print("❌ API Error at: ", Time.get_datetime_string_from_system())
    print("Error: ", error, " (", code, ")")
```

### Support Resources

1. **Plugin Documentation**: Check README.md for basic setup
2. **Test Suite**: Run included tests to verify installation
3. **Settings Panel**: Use enhanced panel for configuration help
4. **Debug Mode**: Enable logging for detailed troubleshooting
5. **itch.io Documentation**: [API documentation](https://itch.io/docs/api/overview)

---

This guide covers all aspects of implementing the GodotItch plugin. For additional support, check the test suite results and enable debug logging to diagnose specific issues.
