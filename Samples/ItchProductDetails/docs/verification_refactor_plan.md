# Verification refactor — focused plan

Purpose
- Provide a single, simple autoload facade named `Itch` (inside the plugin) that handles download-key parsing, validation, verification orchestration, formatting, and error mapping so developers only configure API key and game/app id in project settings.

What to keep (high-value items)
- One autoload: `Itch` that reads `godot_itch.api_key` and `godot_itch.game_id` from project settings and exposes a tiny API for scenes.
- Move all parsing/validation code into plugin core (no example should reimplement it).
- Centralize network calls and error mapping in the plugin; example UIs only call `Itch.verify(...)` and handle three signals.
- Provide formatting helpers for UI (BBCode/plain) but keep UI composition minimal.
- Add unit tests for the validation helpers and a small smoke test for the orchestration.

Minimal public API for `Itch` (facade)
- Methods
  - `func verify(download_input: String) -> void` — uses project settings; emits signals; no API key or game id args required.
  - `func validate(download_input: String) -> Dictionary` — returns `{ valid: bool, key: String, error: String, info: Dictionary }`.
  - `func format_result(result: ItchVerificationResult, as_bbcode: bool = true) -> String` — optional helper.
- Signals
  - `verification_started()`
  - `verification_succeeded(result: ItchVerificationResult)`
  - `verification_failed(error: String)`

Required project settings (documented by plugin)
- `godot_itch.api_key` (string)
- `godot_itch.game_id` (string)

Files to change (minimal list)
- Add `autoload/Itch.gd` (facade autoload) that delegates to existing core modules.
- Update `addons/godot_itch/core/download_key.gd` to expose `extract_and_validate()`.
- Ensure `addons/godot_itch/verification/verification_client.gd` returns `ItchVerificationResult` and is usable by the facade.
- Add `format_bbcode()` to `addons/godot_itch/verification/verification_result.gd` (optional).
- Replace `examples/verification_test.gd` wiring to use `Itch` signals.

Code cleanup suggestions
- Remove UI logic that parses or validates keys from examples; call `Itch.validate()` instead.
- Replace manual signal wiring in examples with single connections to `Itch` in `_ready()`.
- Keep BBCode composition in one place (`format_result`) and keep UI only responsible for display.

Improvements to consider
- Friendly error mapping: map HTTP codes to player-facing messages.
- Add `debug_logging` flag on `Itch` to enable detailed logs during development.
- Optional retry/backoff on transient network errors; surface a simple limit.
- Minimal telemetry hooks (disabled by default) for verification failure categories to help debugging.
- Internationalization readiness: use message keys or helper to format localized messages.

Tests
- Unit tests for `extract_and_validate()` covering raw key, URL, malformed, and empty.
- Mocked verification tests that ensure `verification_succeeded` and `verification_failed` emit expected payloads.

Next steps (recommended)
- Implement the `Itch` autoload facade (delegates to core functions) and update `examples/verification_test.gd` to call `Itch.verify(...)`.
- Add `extract_and_validate()` and small unit tests.

If you want I will implement the autoload facade and update the example now.

Pick which iteration to start and I'll make the edits and run the quick checks.

## Autoload "Itch" design (single-entry autoload)

Goal
- Provide one autoload object named `Itch` (installed by the plugin) that handles validation, orchestration, verification calls, formatting, logging, and signal emission so game developers only need to set API key and App/Game ID in project settings.

Behaviour contract
- Autoload name: `Itch` (script placed at `autoload/Itch.gd` or the plugin registers this autoload when enabled).
- Developer required setup: set two project settings (plugin provides UI instructions)
  - `godot_itch.api_key` (string) — the developer's itch.io API key
  - `godot_itch.game_id` (string) — the itch.io game/app id to verify keys against
- At runtime the player supplies only a download link or raw key string.

Public API (minimal, high-level)
- Methods
  - `func verify(download_input: String) -> void`
    - Reads `godot_itch.api_key` and `godot_itch.game_id` from settings automatically.
    - Performs input validation, emits `verification_started()`, and runs verification.
    - On success emits `verification_succeeded(result: ItchVerificationResult)`.
    - On failure emits `verification_failed(error: String)`.
  - `func validate(download_input: String) -> Dictionary`
    - Returns `{ valid: bool, key: String, error: String, info: Dictionary }` without doing network calls.
  - `func format_result(result: ItchVerificationResult, as_bbcode: bool = true) -> String`
    - Returns a formatted string suitable for UI (BBCode when requested).

- Signals
  - `verification_started()`
  - `verification_succeeded(result: ItchVerificationResult)`
  - `verification_failed(error: String)`

- Properties
  - `debug_logging: bool` — toggles verbose plugin logging

UX details
- If `godot_itch.api_key` or `godot_itch.game_id` are missing: `verify()` should immediately emit `verification_failed` with a clear message telling the developer/player which project setting is missing.
- `verify()` should accept either a raw key or a full itch.io download URL and handle extraction.
- Errors returned by `verification_failed` should be user-friendly (map HTTP 401/403 to "API key invalid", 404 to "key not found or game mismatch", 429 to "rate limited — try again later", etc.).

Implementation notes
- The plugin can ship a thin wrapper autoload at `autoload/Itch.gd` that calls into existing core modules (`download_key.gd`, `itch_api.gd`, `verification_client.gd`) so we don't duplicate logic.
- Keep `Itch` a stable, documented API so example scenes and downstream projects depend only on it.

Files to add/change for this autoload
- Add or update `autoload/Itch.gd` — the facade autoload that implements the public API above.
- Update `addons/godot_itch/core/download_key.gd` to expose `extract_and_validate()` used by `Itch.validate()`.
- Ensure `addons/godot_itch/verification/verification_client.gd` returns `ItchVerificationResult` objects and doesn't assume the caller will manage settings.
- Update `examples/verification_test.gd` to call `Itch.verify(user_input)` and connect to `Itch` signals.

Minimal developer flow
1. Install plugin (it registers autoload `Itch` or plugin activation instructs to add it).
2. Set project settings `godot_itch.api_key` and `godot_itch.game_id`.
3. In any UI scene, call `Itch.verify(download_link_or_key)` when the player submits their download link.
4. Listen to `Itch.verification_succeeded` and `Itch.verification_failed` to update UI.

Next step recommendation
- Implement iteration B (manager orchestration) but make the autoload facade `Itch` the public surface. I can implement the facade and the `verify()` orchestration and then update `examples/verification_test.gd` to use it — if you agree I'll start those edits and run quick checks.




