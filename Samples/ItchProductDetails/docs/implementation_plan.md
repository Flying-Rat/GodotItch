# Implementation Plan: Download Key Verification System

## Overview

This plan extends the existing `godot_itch` addon to include purchase verification using itch.io download keys. The implementation follows a phased approach, building incrementally on the current project structure.

## Current Project Analysis

**Existing Structure:**
```
addons/godot_itch/
├── itch.gd              # Public metadata fetching
├── plugin.gd            # Plugin registration  
└── plugin.cfg           # Plugin configuration

examples/
└── fetch_demo.gd/.tscn  # UI demo for metadata fetching

docs/
├── gdscript_addon_proposal.md
└── purchase_verification_proposal.md
```

**Current Capabilities:**
- Public project metadata fetching via `itch.gd`
- Signal-based async API (`project_fetched`, `fetch_failed`)
- UI demo with simplified interface
- Plugin architecture ready for extension

## Implementation Phases

### Phase 1: Core Verification Infrastructure (Week 1)

**Goal:** Create working prototype with proper plugin structure for verification

**New Plugin Structure:**
```
addons/godot_itch/
├── core/
│   ├── itch_api.gd           # Renamed from itch.gd, focused on API calls
│   └── download_key.gd       # Download key utilities
├── verification/
│   ├── verification_client.gd # Direct itch.io API verification
│   └── verification_result.gd # Data structure for results
├── plugin.gd                 # Plugin registration (ItchAPI autoload only)
└── plugin.cfg                # Plugin configuration
```

**Application Layer (outside plugin):**
```
# Application will create its own autoload for user data
# Plugin only provides ItchAPI functionality
```

**Tasks:**
1. **Restructure existing `itch.gd` → `core/itch_api.gd`**
   - Keep existing public metadata functionality
   - Clean separation of concerns
   - Maintain backward compatibility

2. **Create `core/download_key.gd`** - Download key utilities
   ```gdscript
   class_name ItchDownloadKey
   
   static func extract_from_url(url: String) -> String
   static func validate_format(key: String) -> bool
   static func normalize_key(key: String) -> String
   static func get_key_info(key: String) -> Dictionary
   ```

3. **Create `verification/verification_client.gd`** - Direct API verification
   ```gdscript
   extends HTTPRequest
   class_name ItchVerificationClient
   
   signal verification_completed(result: ItchVerificationResult)
   signal verification_failed(error: String)
   
   func verify_download_key(download_key: String, api_key: String, game_id: String) -> void
   func _build_api_request(key: String, api_key: String, game_id: String) -> Dictionary
   ```

4. **Create `verification/verification_result.gd`** - Result data structure
   ```gdscript
   class_name ItchVerificationResult
   extends Resource
   
   @export var verified: bool = false
   @export var download_key: String = ""
   @export var user_info: Dictionary = {}
   @export var error_message: String = ""
   @export var verified_at: String = ""
   ```

5. **Update `plugin.gd`** - Simple autoload registration
   ```gdscript
   # Only register ItchAPI autoload
   # No user data management in plugin
   ```

**Deliverables:**
- Properly structured plugin with clear separation
- Working prototype that can verify download keys directly with itch.io API
- Clean API for applications to use
- Maintained backward compatibility for existing functionality

### Phase 2: Production-Ready Direct API Integration (Week 2)

**Goal:** Eliminate backend server dependency and create robust direct API solution

**Why No Backend Server:**
- **Simpler architecture**: Direct itch.io API calls from game
- **Reduced infrastructure**: No server hosting/maintenance costs
- **Better reliability**: One less point of failure
- **Easier deployment**: No backend setup required for developers

**Enhanced Plugin Structure:**
```
addons/godot_itch/
├── core/
│   ├── itch_api.gd           # Enhanced with better error handling
│   ├── download_key.gd       # Enhanced validation and parsing
│   └── api_security.gd       # NEW: API key protection utilities
├── verification/
│   ├── verification_client.gd # Enhanced with retry logic, rate limiting
│   ├── verification_result.gd # Enhanced with more metadata
│   └── verification_cache.gd  # NEW: Temporary in-memory caching
└── utils/
    ├── http_helper.gd        # NEW: Robust HTTP request utilities
    └── error_handler.gd      # NEW: Comprehensive error handling
```

**Security Improvements:**
1. **API Key Protection** (`api_security.gd`)
   ```gdscript
   class_name ItchAPISecurity
   
   # Hash API key with game-specific salt
   static func hash_api_key(api_key: String, game_id: String) -> String
   
   # Validate API key format without exposing it
   static func validate_api_key_format(api_key: String) -> bool
   
   # Create secure headers for API requests
   static func create_auth_headers(api_key: String) -> PackedStringArray
   ```

2. **Enhanced Verification Client** (`verification_client.gd`)
   ```gdscript
   extends HTTPRequest
   class_name ItchVerificationClient
   
   # Enhanced with production features
   var max_retries: int = 3
   var retry_delay: float = 1.0
   var timeout: float = 10.0
   
   func verify_download_key(key: String, api_key: String, game_id: String) -> void
   func _handle_rate_limiting(response_code: int) -> void
   func _implement_exponential_backoff() -> void
   func _validate_api_response(data: Dictionary) -> bool
   ```

3. **Temporary Caching** (`verification_cache.gd`)
   ```gdscript
   # Session-only cache (not persistent)
   class_name ItchVerificationCache
   
   static var _cache: Dictionary = {}
   
   static func cache_result(key: String, result: ItchVerificationResult, ttl_minutes: int = 30)
   static func get_cached_result(key: String) -> ItchVerificationResult
   static func clear_cache() -> void
   static func is_cache_valid(key: String) -> bool
   ```

**Direct itch.io API Integration:**
- Use itch.io's `/api/1/key/game/GAME_ID/download_keys` endpoint directly
- Implement proper authentication with developer's API key
- Handle all itch.io API responses and error codes
- Implement rate limiting to respect API limits

**Tasks:**
1. **Enhance verification client with production features**
   - Retry logic with exponential backoff
   - Proper error handling for all API responses
   - Rate limiting and request throttling
   - Timeout handling

2. **Add API security layer**
   - Secure API key handling
   - Request validation
   - Response verification

3. **Implement session caching**
   - In-memory cache for current session
   - Prevent redundant API calls
   - Automatic cache expiration

4. **Add comprehensive error handling**
   - Network connectivity issues
   - Invalid API responses
   - Rate limiting responses
   - Authentication failures

**API Integration Details:**
```gdscript
# Direct itch.io API call pattern
var url = "https://itch.io/api/1/key/game/%s/download_keys" % game_id
var headers = ["Authorization: Bearer %s" % api_key]
var params = "download_key=%s" % download_key

# Expected itch.io API responses:
# Success: {"download_key": {"id": 124, "owner": {...}}}
# Failure: {"errors": ["invalid download key"]}
```

**Deliverables:**
- Production-ready direct API integration
- Robust error handling and retry logic
- Session-based caching system
- Comprehensive API security
- No external server dependencies

### Phase 3: Minimal UI Integration (Week 3)

**Goal:** Create simple, functional verification interface

**Files to Add:**
```
examples/
├── verification_demo.gd     # NEW: Minimal verification UI demo
└── verification_demo.tscn   # NEW: Simple verification scene
```

**Minimal UI Design:**
```
VerificationDemo (Control)
└── VBox
    ├── Title (Label) - "Enter Download Key"
    ├── KeyInput (LineEdit) - Download key or URL input
    ├── VerifyButton (Button) - "Verify"
    ├── StatusLabel (Label) - Status messages
    └── ResultArea (RichTextLabel) - Verification result
```

**Tasks:**
1. **Create simple verification scene**
   - Reuse existing UI patterns from `fetch_demo`
   - Single input field for download key or full URL
   - Basic status display
   - Minimal styling, focus on functionality

2. **Implement verification logic**
   ```gdscript
   extends Control
   
   @onready var key_input: LineEdit = $VBox/KeyInput
   @onready var verify_button: Button = $VBox/VerifyButton
   @onready var status_label: Label = $VBox/StatusLabel
   @onready var result_area: RichTextLabel = $VBox/ResultArea
   
   func _on_verify_pressed():
       var input = key_input.text.strip_edges()
       var download_key = ItchDownloadKey.extract_from_url(input)
       
       if download_key.is_empty():
           show_error("Invalid download key format")
           return
           
       verify_download_key(download_key)
   ```

3. **Integration with plugin**
   - Use ItchAPI autoload
   - Connect to verification signals
   - Handle verification results
   - Show basic user information on success

**UI Features:**
- Auto-extract download key from full itch.io URLs
- Real-time input validation feedback
- Clear success/error states
- Display verified user information

**Deliverables:**
- Working verification UI demo
- Integration with Phase 2 verification system
- Basic user experience
- Foundation for future UI enhancements

### Phase 4: Production Integration with Persistence (Week 4)

**Goal:** Add persistent storage and make system production-ready

**Application Layer Structure:**
```
# Application creates its own autoload for user data management
# This is outside the plugin, in the actual game project

autoload/
├── user_data_manager.gd     # NEW: Application autoload for persistence
└── verification_manager.gd  # NEW: Application verification controller

examples/
└── production_example/      # NEW: Complete game integration example
    ├── main_menu.gd         # With verification gate
    ├── main_menu.tscn
    ├── game_scene.gd        # Protected game content  
    └── game_scene.tscn
```

**Enhanced Plugin (Final Structure):**
```
addons/godot_itch/
├── core/
│   ├── itch_api.gd           # Stable API functionality
│   ├── download_key.gd       # Enhanced utilities
│   └── api_security.gd       # Security utilities
├── verification/
│   ├── verification_client.gd # Production-ready verification
│   ├── verification_result.gd # Enhanced data structure
│   └── verification_cache.gd  # Session caching
├── utils/
│   ├── http_helper.gd        # HTTP utilities
│   └── error_handler.gd      # Error handling
├── plugin.gd                 # Plugin registration
└── plugin.cfg                # Plugin configuration
```

**Tasks:**
1. **Create application-level persistence** (`user_data_manager.gd`)
   ```gdscript
   extends Node  # Application autoload, not plugin
   
   const SAVE_FILE = "user://verification_data.save"
   
   func save_verification(result: ItchVerificationResult) -> void
   func load_verification() -> ItchVerificationResult
   func clear_verification() -> void
   func is_verification_valid() -> bool
   func get_verification_expiry() -> String
   ```

2. **Create verification manager** (`verification_manager.gd`)
   ```gdscript
   extends Node  # Application autoload
   
   signal verification_completed(success: bool)
   signal verification_failed(error: String)
   
   var api_key: String = ""  # Set by developer
   var game_id: String = ""
   var require_verification: bool = true
   
   func setup(config: Dictionary) -> void
   func check_verification_status() -> bool
   func start_verification_process() -> void
   func verify_download_key(key: String) -> void
   ```

3. **Create production example**
   - Main menu with verification gate
   - Automatic verification check on startup
   - Graceful handling of verification states
   - Protected game content behind verification

4. **Add developer configuration**
   - Simple resource-based configuration
   - Clear setup documentation
   - Example configuration files

**Persistence Features:**
- Encrypted save file for verification data
- Configurable expiration time (default: 7 days)
- Tamper detection for save files
- Graceful handling of corrupted saves

**Developer Setup:**
```gdscript
# In your game's main scene _ready()
func _ready():
    VerificationManager.setup({
        "api_key": "your_itch_api_key",
        "game_id": "12345", 
        "require_verification": true,
        "cache_duration_days": 7
    })
    
    if VerificationManager.check_verification_status():
        start_game()
    else:
        show_verification_ui()
```

**Production Features:**
- Automatic verification persistence
- Configurable cache duration
- Offline mode for verified users
- Clean separation between plugin and application logic

**Deliverables:**
- Complete verification system with persistence
- Application-level autoloads for user data
- Production-ready example implementation
- Developer setup documentation
- Clear separation between plugin and application concerns

### Phase 5: Advanced Features (Optional - Future Enhancement)

**Goal:** Add polish and advanced functionality when needed

**Note:** This phase is optional and can be implemented later based on user feedback and requirements.

**Potential Advanced Features:**
1. **Automatic key extraction from installation**
2. **Enhanced offline verification with cryptographic tokens**
3. **Multi-platform credential storage integration**
4. **Advanced analytics and monitoring (privacy-respecting)**
5. **Enhanced UI with animations and better UX**
6. **Multiple verification methods (email fallback, etc.)**

**Implementation:** To be planned based on Phase 1-4 feedback and real-world usage.

## Configuration for Developers

### Setup Requirements

**Developer Configuration:**
```gdscript
# In your game's main scene _ready() function
func _ready():
    # Configure verification system (application-level)
    VerificationManager.setup({
        "api_key": "your_itch_api_key",      # Your itch.io API key
        "game_id": "12345",                  # Your itch.io game ID
        "require_verification": true,         # Mandatory verification
        "cache_duration_days": 7             # How long to cache verification
    })
    
    # Check verification status
    if VerificationManager.check_verification_status():
        start_game()
    else:
        show_verification_ui()
```

**Recommended File Structure:**
```
your_game/
├── autoload/
│   ├── user_data_manager.gd      # Application autoload for persistence
│   └── verification_manager.gd   # Application verification controller
├── ui/
│   ├── verification_scene.tscn   # Verification UI
│   └── verification_scene.gd     # Verification controller
├── game/
│   └── main_game.tscn           # Your actual game
└── main.gd                      # Entry point with verification check
```

## Security Considerations

1. **API Key Protection**
   - Store API key securely in your application code
   - Consider obfuscation for production builds
   - Never expose API key in logs or debug output

2. **Verification Caching**
   - Time-limited cache (configurable, default 7 days)
   - Encrypted local storage using Godot's built-in encryption
   - Tamper detection for save files

3. **Direct API Security**
   - HTTPS-only communication with itch.io
   - Proper request validation
   - Rate limiting to respect itch.io API limits
   - Timeout handling for network issues

4. **Client Security**
   - Input validation and sanitization
   - Protection against malformed download keys
   - Graceful error handling

## Why No Backend Server?

**Advantages of Direct API Integration:**
1. **Simplicity**: No server infrastructure to maintain
2. **Cost**: No hosting costs for verification server
3. **Reliability**: One less point of failure
4. **Deployment**: Easier for developers to implement
5. **Latency**: Direct connection to itch.io (no middleware)

**Security Considerations Addressed:**
- API key protection through application-level security
- Direct HTTPS communication with itch.io
- Rate limiting and proper error handling
- Session-based caching to minimize API calls

## Testing Strategy

### Phase 1-2 Testing
- Unit tests for key parsing and validation
- Mock server for development testing
- Integration tests with real itch.io API

### Phase 3-4 Testing
- UI/UX testing with real users
- Cross-platform testing
- Performance testing under load

### Phase 5 Testing
- Security penetration testing
- Stress testing with high verification volumes
- Long-term reliability testing

## Success Metrics

1. **Technical Metrics**
   - Verification success rate > 95%
   - Average verification time < 3 seconds
   - Cache hit rate > 80%

2. **User Experience Metrics**
   - User drop-off rate during verification < 5%
   - Support tickets related to verification < 1%
   - Positive user feedback on verification process

3. **Developer Metrics**
   - Setup time for new developers < 30 minutes
   - Documentation completeness score > 90%
   - Community adoption rate

## Risk Mitigation

1. **Server Downtime**
   - Graceful fallback to cached verification
   - Multiple server endpoints for redundancy
   - Clear user communication during outages

2. **API Changes**
   - Version your verification API
   - Backward compatibility for at least 6 months
   - Monitoring for itch.io API changes

3. **User Experience Issues**
   - Comprehensive error messages
   - Multiple verification methods
   - Customer support integration

This implementation plan provides a structured approach to building a robust download key verification system while maintaining the quality and usability of the existing codebase.
