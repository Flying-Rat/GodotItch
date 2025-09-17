## GodotItch v1.0.0 — Initial release

A lightweight Godot plugin that verifies itch.io download keys against the itch.io API so you can validate purchases in-game and protect builds.

### Highlights
- Reliable download key validation against the itch.io API  
- Async API with signals for non-blocking verification  
- Supports raw keys, full URLs, and download URLs with key params  
- In-editor settings panel and automatic `Project Settings` integration:
  - `godot_itch/api_key`
  - `godot_itch/game_id`
  - `godot_itch/require_verification`  
- Debug logging and robust error handling  
- Production-ready test suite (36 tests, 100% coverage)

### Quick start
1. Copy `addons/godot_itch` into your project’s `addons/` folder.  
2. Enable the plugin in Project Settings → Plugins and restart Godot.  
3. Configure `godot_itch/api_key` and `godot_itch/game_id` in Project Settings.

### Tiny usage example
```gdscript
# gdscript
GodotItch.connect_verification_completed(_on_verified)
GodotItch.connect_verification_failed(_on_failed)
GodotItch.verify("download_key_here")
```

### Testing
```bash
# run tests (headless)
godot --headless tests/final_validation.tscn
```

Notes: Initial public release — all tests passing and ready for production. See `README.md` for full API, troubleshooting, and configuration details.
