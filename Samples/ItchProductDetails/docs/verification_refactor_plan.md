# GodotItch Plugin - Complete Design & Architecture Proposal

## Vision & Goals

**Primary Goal**: Create the most developer-friendly itch.io integration plugin for Godot that requires minimal setup and zero boilerplate code.

**Core Principles**:
- **Zero Configuration Complexity**: One-click plugin activation with visual project settings
- **Minimal API Surface**: Single autoload with 3-4 methods maximum
- **Invisible Complexity**: All network, parsing, validation, and error handling hidden from developer
- **Production Ready**: Built-in caching, retry logic, and user-friendly error messages
- **Extensible Foundation**: Clean architecture for future itch.io features

## Developer Experience (The 5-Minute Setup)

### Step 1: Plugin Installation
```
1. Download/clone plugin to addons/godot_itch/
2. Project Settings > Plugins > Enable "GodotItch"
3. Plugin automatically registers "Itch" autoload
```

### Step 2: Configuration (Visual UI)
```
1. Project Settings > GodotItch section appears automatically
2. Fill in two fields:
   - API Key: [Password field with "Get API Key" button]
   - Game ID: [Text field with "Find My Game ID" helper]
3. Optional settings have smart defaults
```

### Step 3: Implementation (2 lines of code)
```gdscript
# In your main scene
func _ready():
    Itch.verification_completed.connect(_on_verified)
    # That's it! Plugin handles everything else

func _on_download_key_entered(key: String):
    Itch.verify(key)  # One method call

func _on_verified(user_info: Dictionary):
    start_game(user_info)  # Clean, simple data
```

## Unified Plugin Architecture

### Single Entry Point: `Itch` Autoload
```gdscript
# Global autoload - automatically registered by plugin
extends Node

# ============ PUBLIC API (Only these 4 methods) ============

## Verify a download key/URL - handles everything automatically
func verify(download_input: String) -> void

## Quick validation without network call
func validate(download_input: String) -> Dictionary

## Get current verification status (for game flow logic)
func get_verification_status() -> Dictionary

## Clear cached verification (for logout/account switching)
func clear_verification() -> void

# ============ SIGNALS (Clean, semantic) ============
signal verification_started()  # Show loading UI
signal verification_completed(user_info: Dictionary)  # Success!
signal verification_failed(error_message: String, error_code: String)

# ============ PROPERTIES (Optional configuration) ============
var debug_mode: bool = false  # Enable verbose logging
var offline_mode: bool = false  # Use cached results only
```

### Intelligent Project Settings Integration

**Enhanced Plugin.gd**:
```gdscript
@tool
extends EditorPlugin

func _enter_tree():
    _create_project_settings()
    _register_autoload()
    _show_welcome_message()

func _create_project_settings():
    # Smart API key field with helper buttons
    _add_setting("godot_itch/api_key", "", {
        "type": TYPE_STRING,
        "hint": PROPERTY_HINT_PASSWORD,
        "description": "Your itch.io API key for verification"
    })
    
    # Game ID with auto-detection helper
    _add_setting("godot_itch/game_id", "", {
        "type": TYPE_STRING,
        "hint": PROPERTY_HINT_PLACEHOLDER_TEXT,
        "hint_string": "e.g., 1234567 (find in your game's URL)",
        "description": "Your game's ID from itch.io"
    })
    
    # Optional: Enable debug logging
    _add_setting("godot_itch/debug_logging", false, {
        "type": TYPE_BOOL,
        "description": "Enable detailed logging"
    })

func _show_welcome_message():
    print("[GodotItch] Plugin enabled! Configure in Project Settings > GodotItch")
    print("[GodotItch] Docs: https://github.com/your-repo/godot-itch/wiki")
```

## Advanced Core Features

### 1. Smart Input Processing
```gdscript
# In core/input_processor.gd
class_name ItchInputProcessor

## Handles all possible input formats intelligently
static func process_input(raw_input: String) -> Dictionary:
    # URL patterns: itch.io, itch.zone, custom domains
    # Raw key patterns: various formats and lengths
    # QR code data: embedded JSON or plain keys
    # Returns: { valid: bool, key: String, format: String, metadata: Dictionary }
```

### 2. Simple Network Layer
```gdscript
# In core/network_manager.gd
class_name ItchNetworkManager

## Basic networking with standard error handling
func verify_key_async(key: String, api_key: String, game_id: String) -> Dictionary:
    # Direct HTTP request to itch.io API
    # Standard HTTP error codes returned
    # Basic timeout handling
    # Returns raw API response format
```

### 3. Basic Error Handling
```gdscript
# In core/error_handler.gd
class_name ItchErrorHandler

## Simple error handling - technical messages only
static func handle_error(response_code: int, error_details: String) -> String:
    # Return technical error descriptions
    # HTTP status codes and API error messages
    # No translation or user-friendly formatting
```

## Plugin File Structure (Reorganized)

```
addons/godot_itch/
├── plugin.cfg                          # Plugin metadata
├── plugin.gd                           # Enhanced editor plugin
├── autoload/
│   └── itch.gd                         # Main autoload facade (THE public API)
├── core/
│   ├── input_processor.gd               # Smart input parsing and validation
│   ├── network_manager.gd               # Basic HTTP client
│   ├── error_handler.gd                 # Technical error handling
│   └── config_manager.gd                # Project settings management
├── api/
│   ├── verification_client.gd           # itch.io API client
│   ├── metadata_client.gd               # Game metadata fetching
│   └── api_types.gd                     # Shared data structures
├── tests/
│   ├── test_input_processor.gd          # Unit tests for input parsing
│   ├── test_network_manager.gd          # Mock API tests
│   └── test_integration.gd              # End-to-end tests
└── docs/
    ├── README.md                        # Quick start guide
    └── API_REFERENCE.md                 # Complete API documentation
```

## Core Features & Improvements

### 1. Basic Game Flow Integration
```gdscript
# Simple verification check
func _ready():
    if Itch.is_verification_required():
        show_verification_ui()
    else:
        start_game()

func handle_verification_flow():
    # Basic states: verified, required
    if Itch.get_verification_status().verified:
        start_game()
    else:
        show_verification_ui()
```

### 2. Debug Tools
```gdscript
# Debug mode for development
func _ready():
    if OS.is_debug_build():
        Itch.debug_mode = true  # Shows technical logs
```

## Practical Implementation Plan

### Phase 1: Core Refactor (Week 1)
**Goal**: Create minimal but functional `Itch` autoload

**Tasks**:
1. **Create new autoload structure**
   - Add `autoload/itch.gd` with 4 core methods
   - Implement basic `verify()`, `validate()`, `get_verification_status()`, `clear_verification()`
   - Add 3 essential signals: `verification_started`, `verification_completed`, `verification_failed`

2. **Update plugin registration**
   - Modify `plugin.gd` to register `Itch` autoload instead of `ItchAPI`
   - Keep existing project settings but add `debug_logging`
   - Remove complex settings (caching, retry, friendly errors)

3. **Create input processor**
   - Move URL/key parsing logic from examples into `core/input_processor.gd`
   - Handle basic URL formats and raw keys
   - Return simple validation dictionary

4. **Update example to use new API**
   - Replace `verification_test.gd` to use `Itch.verify(key)`
   - Connect to `Itch` signals instead of direct client usage
   - Remove manual validation and error handling from UI

**Deliverables**:
- Working `Itch` autoload with minimal API
- Updated example scene using new API
- Basic input validation moved to core

### Phase 2: Network & Error Handling (Week 2)
**Goal**: Robust networking with technical error reporting

**Tasks**:
1. **Implement network manager**
   - Create `core/network_manager.gd` for HTTP requests
   - Basic timeout handling (no retries)
   - Standard HTTP error code handling

2. **Technical error handling**
   - Create `core/error_handler.gd` for technical error messages
   - Map HTTP codes to technical descriptions
   - Return raw API error messages

3. **Integrate with verification client**
   - Update existing `verification_client.gd` to use new network manager
   - Ensure it works with `Itch` autoload
   - Keep existing result format

4. **Add unit tests**
   - Test input processing with various formats
   - Mock network tests for API calls
   - Basic integration tests

**Deliverables**:
- Reliable network layer with technical error handling
- Updated verification client integration
- Basic test coverage

### Phase 3: Polish & Documentation (Week 3)
**Goal**: Production-ready plugin with clear documentation

**Tasks**:
1. **Debug and logging improvements**
   - Implement debug mode in `Itch` autoload
   - Add detailed logging for development
   - Error logging for troubleshooting

2. **Configuration management**
   - Create `core/config_manager.gd` for project settings
   - Centralize all settings access
   - Add validation for required settings

3. **Documentation**
   - Create comprehensive `README.md`
   - Write `API_REFERENCE.md` with examples
   - Update code comments and documentation

4. **Final testing and cleanup**
   - Test all examples work with new system
   - Remove deprecated code
   - Performance optimization

**Deliverables**:
- Complete, documented plugin
- Clean, maintainable codebase
- Ready for production use

## Migration Strategy

### Step 1: Backward Compatibility
- Keep existing autoloads temporarily
- Add deprecation warnings
- Provide migration guide

### Step 2: Gradual Migration
- Update examples to use new API
- Test both old and new systems work
- Document breaking changes

### Step 3: Clean Migration
- Remove old autoloads
- Update all references
- Clean up unused code

## Success Metrics

**Developer Adoption**:
- Setup time: < 5 minutes from download to working verification
- Lines of code required: < 10 lines for basic implementation
- Configuration complexity: 2 required settings (API key, Game ID)

**Technical Requirements**:
- Network reliability: Direct HTTP requests with basic error handling
- Error clarity: Technical HTTP status codes and API error messages
- Performance: Direct API calls without caching overhead

**Maintainability**:
- Test coverage: Basic unit tests for core functionality
- Documentation: Clear API reference with examples
- Code simplicity: Minimal dependencies and straightforward logic

This simplified design focuses on creating a practical, minimal itch.io integration that developers can easily understand and modify. The architecture is straightforward while still providing a clean separation between the public API and internal implementation details.




