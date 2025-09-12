# GodotItch — Proposal

Purpose

This document briefly captures the goals and high-level proposal for the GodotItch GDExtension.

Goals
- Provide a small, well-documented Godot extension that integrates itch.io functionality (verification, downloads, metadata) in a secure, cross-platform way.
- Follow Godot idioms: `user://` persistence, GDExtension bindings, Callable-based async APIs.
- Make the entitlements/verification path safe for production: local cache + recommended server-authoritative verification.
- Keep the extension modular so consumers can adopt parts independently.

High-level modules
- Core — networking, config, lifecycle, persistence (ItchDataStore)
- User — account/session helpers
- Games — game metadata and uploads
- Entitlements — verification & cached entitlements (primary playtime-check module)
- Assets — file downloads and caching

Naming convention
- Use `*_subsystem` for module names (for example `user_subsystem`, `games_subsystem`, `assets_subsystem`) instead of `*_manager`. This keeps naming consistent and avoids the overloaded `Manager` suffix.

API surface & discoverability
- Expose a native facade `Itch` with module properties (e.g. `Itch.entitlements`) so GDScript users call `Itch.entitlements.verify_purchase(...)`.

Repository layout (short)
- `src/` — C++ sources organized by module (bindings/, core/, entitlements/, games/, user/, assets/)
- `DOCS/` — docs split into Proposal, Plan, API, Modules, Examples
- `demo/` — a minimal Godot project demonstrating the extension
- `tests/` — unit and integration tests

Security notes
- Do not store secrets in repository. Prefer server-authoritative verification for production.
- Persist user data in `user://` and encrypt where appropriate using Godot `FileAccess` encrypted APIs.

Status
- This proposal consolidates decisions made during the refactor: JSON-based encrypted persistence, ItchDataStore as the persistence core, and a clear entitlements facade.

Next
- See `PLAN.md` for an actionable migration and implementation plan.
