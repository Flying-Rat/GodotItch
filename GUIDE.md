# Itch Plugin - Developer Guide

Quick implementation guide for itch.io purchase verification.

## Setup

### Installation
1. Copy `addons/godot_itch` to your project
2. Enable plugin in **Project Settings > Plugins**
3. Restart Godot

### Configuration
Set in **Project Settings**:
- `godot_itch/api_key`: Your itch.io API key
- `godot_itch/game_id`: Your game's ID
- `godot_itch/require_verification`: true

## Basic Implementation

```gdscript
extends Control

func _ready():
    Itch.verification_completed.connect(_on_verified)
    Itch.verification_failed.connect(_on_failed)

func verify_purchase(key: String):
    Itch.verify(key)

func _on_verified(user_info: Dictionary):
    print("User verified: ", user_info.display_name)
    unlock_premium_content()

func _on_failed(error: String, code: String):
    print("Failed: ", error)
    show_error_message(error)
```

## Complete UI Example

```gdscript
extends Control

@onready var key_input: LineEdit = $KeyInput
@onready var verify_button: Button = $VerifyButton
@onready var status_label: Label = $StatusLabel

func _ready():
    verify_button.pressed.connect(_on_verify_pressed)
    Itch.verification_completed.connect(_on_verified)
    Itch.verification_failed.connect(_on_failed)

func _on_verify_pressed():
    var key = key_input.text.strip_edges()
    if key.is_empty():
        status_label.text = "Please enter a download key"
        return
    
    verify_button.disabled = true
    status_label.text = "Verifying..."
    Itch.verify(key)

func _on_verified(user_info: Dictionary):
    verify_button.disabled = false
    status_label.text = "✅ Verified: " + user_info.display_name
    unlock_game()

func _on_failed(error: String, code: String):
    verify_button.disabled = false
    status_label.text = "❌ " + error
```

## Input Validation

```gdscript
# Validate before verification
func validate_input(input: String) -> bool:
    var validation = Itch.validate(input)
    if not validation.valid:
        show_error(validation.error)
        return false
    return true
```

## Best Practices

### Performance
- Cache verification results locally
- Don't verify on every game start
- Use async signals, don't block UI

### Security  
- Always validate input format
- Store verification status securely
- Implement rate limiting

### User Experience
- Provide clear error messages
- Show loading states during verification
- Offer offline/demo mode for failures

## Troubleshooting

**Common Issues:**
1. "Plugin not enabled" → Enable and restart Godot
2. "API key not set" → Configure in Project Settings  
3. "Invalid key format" → Keys are 20-64 chars, alphanumeric + underscores

**Debug Mode:**
Set `godot_itch/debug_logging = true` for detailed logging.

## Testing
Run included tests:
```bash
godot --headless tests/final_validation.tscn
```

---
For complete API reference, see README.md
