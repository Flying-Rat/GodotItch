# GodotItch — API Reference (summary)

This file summarizes the public API surface that modules should expose to GDScript and C++ consumers. Keep signatures Godot-friendly.

Core
- initialize(String api_base_url = "https://itch.io/api/1") -> bool
- shutdown() -> void
- set_api_key(String key) -> void
- get_api_key() -> String
- request(String method, String path, Dictionary params, Callable on_success, Callable on_error) -> void
- get_persistent_store() -> ItchDataStore

ItchDataStore (persistence)
- static ItchDataStore *get_singleton()
- bool initialize(String filename = "user://itch_persistent.dat", String password = "")
- bool is_verified(String key)
- Dictionary get_verification_record(String key)
- void set_verification_record(String key, Dictionary record)
- void clear_verification(String key)
- void clear_all()
- bool save_to_disk()

Entitlements
- void verify_entitlement(String platform, String identifier, Callable on_result, Callable on_error)
- bool is_entitled(String platform, String identifier)
- Dictionary get_entitlement_record(String platform, String identifier)
- void clear_entitlement(String platform, String identifier)
- void clear_all_entitlements()

Games
- void fetch_owned_games(Callable on_success, Callable on_error)
- void fetch_game_info(int game_id, Callable on_success, Callable on_error)
- void fetch_uploads(int game_id, Callable on_success, Callable on_error)

User
- Dictionary get_current_user()
- void fetch_current_user(Callable on_success, Callable on_error)
- bool is_authenticated()
- void clear_session()

Assets
- void download_asset(String url, String target_filename, Callable on_progress, Callable on_success, Callable on_error)
- bool is_asset_cached(String target_filename)
- String get_asset_local_path(String target_filename)
- void clear_asset_cache()

Data shapes
- VerificationRecord: { verified: bool, timestamp: int, meta: Dictionary, source: String, expires_at?: int }
- Game Dictionary: { id: int, title: String, url: String, uploads: Array }

Error shape
- { code: int, message: String, details: Dictionary }

Notes
- Prefer Callables for async results. Return typed Dictionary/Array over JSON strings for ease of use in GDScript.
- Keep method names stable; newer features should add overloads rather than change existing signatures.

Naming convention
- Use the suffix `subsystem` for core modules (e.g. `user_subsystem`, `games_subsystem`, `assets_subsystem`) instead of `manager` or `service`. This keeps names consistent and avoids the overused `Manager` term.
