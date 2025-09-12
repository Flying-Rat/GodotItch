# GodotItch — Implementation Plan

This plan breaks the work into concrete phases and tasks so you can migrate and extend the codebase with minimal risk.

Phases

1) Stabilize persistence (core)
- Move `itch_data_store.*` to `src/core/persistent/` and add atomic save behavior.
- Add unit tests for read/write and encrypted roundtrip.

2) Create Entitlements facade
- Add `src/entitlements/entitlements.h/.cpp` that exposes `verify_entitlement(...)` and local cache checks.
- Move verification logic from `godotitch.cpp` into the new facade.

3) Bindings & facade exposure
- Add `src/bindings/itch.h/.cpp` (or reuse `godotitch.*`) as the C++ `Itch` facade exposing `entitlements`, `core`, etc.
- Update `register_types.cpp` to register `Itch` instance as a global singleton available to GDScript.

4) Module scaffolding
- Add `user/`, `games/`, `assets/` module stubs and basic request wrappers.
- Implement `core/http_client` wrapper to centralize HTTPRequest usage and error normalization.

5) Tests & demo
- Add unit tests for `ItchDataStore` and `Entitlements` (happy path + cache TTL + error path).
- Add a small demo script in `demo/` showing `Itch.entitlements.verify_purchase` usage.

6) CI and packaging
- Add GitHub Actions to build SCons targets and run unit tests.
- Add packaging scripts to produce per-platform release artifacts.

Risk & mitigations
- API surface changes: keep `ItchDataStore` header stable and write adapter shims when moving files.
- Platform differences (Web/HTML5): test `user://` behavior on web early using CI and demo.
- Secrets: never add server keys to the repo. Use environment variables or CI secrets.

Acceptance criteria
- All new headers compile without changing public signatures used by the demo.
- Itch entitlements verification returns cached results when valid and falls back to server when missing.
- Unit tests covering persistence and verification logic pass in CI.

Estimate & next steps
- Stabilize persistence + entitlements facade: 1–2 working days.
- Bindings & demo: 1 day.
- Tests + CI: 1 day.

If you'd like, I can scaffold the initial files for phase (1) and (2) now.

---

## Tasks (breakdown)

This section breaks the plan into small, actionable tasks you can copy into your issue tracker. Each task includes a short ETA, files to create/edit, and dependencies.

Project modules considered:
- core (persistence, HTTP client)
- entitlements (verification + cache)
- user (account/session)
- games (game metadata)
- assets (downloads / caching)
- bindings (Itch facade, register_types)
- utils, tests, ci, docs

Tasks are grouped by phase from the plan.

Phase 1 — Stabilize persistence (core)
- TASK-001: Move ItchDataStore into core/persistent
	- ETA: 1–2 hours
	- Files: move `src/itch_data_store.h`, `src/itch_data_store.cpp` -> `src/core/persistent/`
	- Update: include paths in `godotitch.cpp`, `register_types.cpp`
	- Dependencies: none

- TASK-002: Add atomic save helpers
	- ETA: 2–3 hours
	- Files: `src/core/persistent/local_crypto.cpp/.h` (new), modify `itch_data_store.cpp` to write tmp file + rename
	- Dependencies: TASK-001

- TASK-003: Unit tests for persistence
	- ETA: 2–3 hours
	- Files: `tests/unit/test_itch_data_store.cpp` (new)
	- Dependencies: TASK-001, TASK-002

- TASK-001A: Scaffold Core module API (core init + config)
	- ETA: 1–2 hours
	- Files: `src/core/core.h`, `src/core/core.cpp`
	- Responsibilities: provide `initialize()`, `shutdown()`, `set_api_key()`, `get_api_key()`, and `get_persistent_store()` signatures. Wire minimal implementations that call into `ItchDataStore` where needed.
	- Dependencies: TASK-001

Phase 2 — Create Entitlements facade
- TASK-004: Add entitlements facade (header + cpp)
	- ETA: 3–4 hours
	- Files: `src/entitlements/entitlements.h`, `src/entitlements/entitlements.cpp`
	- Responsibilities: expose `verify_entitlement`, `is_entitled`, `get_entitlement_record`, cache logic calling ItchDataStore
	- Dependencies: TASK-001

- TASK-005: Move verification logic from `godotitch.cpp`
	- ETA: 1–2 hours
	- Files: edit `src/godotitch.cpp`, add calls into Entitlements facade
	- Dependencies: TASK-004

- TASK-006: Add entitlements adapter for itch (internal)
	- ETA: 2 hours
	- Files: `src/entitlements/adapters/entitlements_itch.h`, `entitlements_itch.cpp` (stubs implementing current logic)
	- Dependencies: TASK-004

- TASK-007: Unit tests for entitlements caching/TTL
	- ETA: 2–3 hours
	- Files: `tests/unit/test_entitlements.cpp`
	- Dependencies: TASK-004, TASK-006, TASK-003

Phase 3 — Bindings & facade exposure
- TASK-008: Implement C++ `Itch` facade and expose modules
	- ETA: 2–3 hours
	- Files: `src/bindings/itch.h`, `src/bindings/itch.cpp` (or reuse `godotitch.*`), modify `register_types.cpp`
	- Outcome: GDScript can call `Itch.entitlements.verify_purchase(...)`
	- Dependencies: TASK-004, TASK-006

- TASK-009: Add small GDScript autoload shim (optional)
	- ETA: 30–60 minutes
	- Files: `demo/Itch.gd` (or `res://addons/godot_itch/Itch.gd`) — constructs the C++ facade if needed
	- Dependencies: TASK-008 (optional alternative)

Phase 4 — Module scaffolding (user, games, assets)
- TASK-010: Add user module stubs
	- ETA: 2 hours
	- Files: `src/user/user_subsystem.h`, `src/user/user_subsystem.cpp`
	- Dependencies: TASK-008

- TASK-010A: Implement User module methods
	- ETA: 4 hours
	- Files to edit: `src/user/user_subsystem.cpp`
	- Responsibilities: implement `get_current_user()`, `fetch_current_user()`, `is_authenticated()`, `clear_session()`. Persist session tokens to `ItchDataStore` if applicable and normalize error callbacks.
	- Dependencies: TASK-008, TASK-013

- TASK-011: Add games module stubs
	- ETA: 2 hours
	- Files: `src/games/games_subsystem.h`, `src/games/games_subsystem.cpp`
	- Dependencies: TASK-008

- TASK-011A: Implement Games module methods
	- ETA: 4 hours
	- Files to edit: `src/games/games_subsystem.cpp`
	- Responsibilities: implement `fetch_owned_games()`, `fetch_game_info()`, `fetch_uploads()` using `core/http_client`. Normalize responses into Godot-friendly Dictionaries/Arrays.
	- Dependencies: TASK-008, TASK-013

- TASK-012: Add assets module stubs
	- ETA: 3 hours
	- Files: `src/assets/assets_subsystem.h`, `src/assets/assets_subsystem.cpp`
	- Dependencies: TASK-008

- TASK-012A: Implement Assets module methods
	- ETA: 4 hours
	- Files to edit: `src/assets/assets_subsystem.cpp`
	- Responsibilities: implement `download_asset()`, `is_asset_cached()`, `get_asset_local_path()`, `clear_asset_cache()`. Use streaming-to-disk and report progress via Callables.
	- Dependencies: TASK-013

- TASK-013: Implement core/http_client wrapper
	- ETA: 3–4 hours
	- Files: `src/core/http_client.h`, `src/core/http_client.cpp`
	- Purpose: centralize HTTPRequest creation, normalize errors, add timeouts and retries
	- Dependencies: TASK-001

Phase 5 — Tests, demo and polish
- TASK-014: Demo script for verify_purchase
	- ETA: 1–2 hours
	- Files: `demo/verify_demo.gd` using `Itch.entitlements.verify_purchase`
	- Dependencies: TASK-008, TASK-004

- TASK-015: Integration test (headless) executing demo script
	- ETA: 3–4 hours
	- Files: `tests/integration/demo_verify_test/` (scripts + CI invocation)
	- Dependencies: TASK-014

Phase 6 — CI and packaging
- TASK-016: Add GitHub Actions workflow (build + unit tests)
	- ETA: 3–6 hours
	- Files: `.github/workflows/build-test.yml`
	- Steps: scons build matrix, run unit tests, upload artifacts
	- Dependencies: TASK-003, TASK-007

- TASK-017: Packaging script for per-platform artifacts
	- ETA: 2–3 hours
	- Files: `packaging/package_release.ps1` (and linux/mac sh scripts)
	- Dependencies: TASK-016

Backlog / Nice-to-have
- TASK-018: Add server-side verification API spec and example server (tiny Flask or Node example)
	- ETA: 4–8 hours
	- Files: `tools/verify_server_example/`

- TASK-019: Add stronger key management notes and key-rotation tooling
	- ETA: 4 hours

How to use this task list
- Start with Phase 1 tasks (TASK-001 to TASK-003) to stabilize persistence.
- Then implement Entitlements (TASK-004..TASK-007) and the facade (TASK-008).
- Iterate on other modules and tests.

If you'd like I can scaffold TASK-001, TASK-004, and TASK-008 now (move `itch_data_store.*`, create `entitlements` facade stubs, and add `bindings/itch.*` facade) — tell me which exact tasks to run and I'll make the changes.
