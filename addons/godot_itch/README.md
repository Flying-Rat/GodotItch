# GodotItch Plugin

A Godot 4 plugin + GDExtension to interact with the itch.io API from your game. It supports authentication via the itch app (launcher API key) or OAuth and exposes high-level calls through the `Itch` facade.

## 🎮 Demo

- Load `res://addons/godot_itch/godot_itch_showcase.tscn` for a minimal UI demonstrating core functionality.
- Additional focused test scenes are available in the project under `res://tests/` (see Testing below).

## Features

- OAuth helper (open browser + save token)
- Launcher detection (uses `ITCHIO_API_KEY` when launched from the itch app)
- Credentials info: `Itch.get_credentials_info()`
- User profile: `Itch.get_me()` (requires `profile:me` scope)
- Async API with signals (`api_response`, `api_error`)
- Project Settings integration

## Installation

1. Copy `addons/godot_itch` into your project’s `addons/` directory.
2. Enable the plugin in Project Settings → Plugins.
3. Restart the editor.

## Configuration

Configure in Project Settings → `godot_itch`:

- `oauth_client_id` — OAuth client ID from your itch developer settings
- `oauth_redirect_uri` — Redirect URI (leave blank to use out-of-band: `urn:ietf:wg:oauth:2.0:oob`)
- `oauth_scope` — Set to `profile:me` (currently the only supported scope)

Notes:
- If launched via the itch app, the plugin will use the provided short-lived launcher token automatically.
- For OAuth, you must start the flow, then paste and save the token to make authenticated calls.

## Quick Start

```gdscript
extends Control

func _ready() -> void:
    # 1) Initialize HTTPRequest usage
    Itch.initialize_with_scene(self)

    # 2) Connect signals
    Itch.api_response.connect(_on_api_response)
    Itch.api_error.connect(_on_api_error)

    # 3) Authenticate (either via itch launcher or OAuth)
    # Itch.get_auth().start_oauth_authorization("<client_id>", "<redirect_uri>", "state")
    # Itch.get_auth().set_oauth_token("<pasted_token>")

    # 4) Call endpoints
    Itch.get_credentials_info()
    Itch.get_me()

func _on_api_response(endpoint: String, data: Dictionary) -> void:
    match endpoint:
        "credentials_info":
            var scopes = data.get("scopes", [])
            var expires_at = str(data.get("expires_at", ""))
            print("Scopes:", scopes)
            if not expires_at.is_empty():
                print("JWT expires at:", expires_at)
        "get_me":
            var user = data.get("user", {})
            print("User:", user.get("username", "?"), user.get("display_name", "?"))

func _on_api_error(endpoint: String, error_message: String, response_code: int) -> void:
    push_error("[%s] (%d) %s" % [endpoint, response_code, error_message])
```

The plugin automatically selects the correct server path for your credential type:
- API key → `/api/1/key/...`
- OAuth JWT → `/api/1/jwt/...`
In both cases, the token is sent in `Authorization: Bearer <token>`.

## API Reference (Plugin Facade)

- `Itch.initialize_with_scene(scene_node: Node)` — Attach internal HTTPRequest to your scene.
- `Itch.get_credentials_info()` — Calls `/api/1/{key|jwt}/credentials/info`, emits `api_response("credentials_info", data)`.
- `Itch.get_me()` — Calls `/api/1/{key|jwt}/me`, emits `api_response("get_me", data)`.

Auth helpers via `Itch.get_auth()`:
- `set_oauth_client_id(client_id: String)`
- `set_oauth_redirect_uri(redirect_uri: String)`
- `set_oauth_scope(scope: String)` (use `profile:me`)
- `start_oauth_authorization(client_id: String = "", redirect_uri: String = "", state: String = "")`

Signals:
- `api_response(endpoint: String, data: Dictionary)`
- `api_error(endpoint: String, error_message: String, response_code: int)`

## Testing

- Open and run these focused scenes in the editor:
  - `res://tests/credentials_info_test.tscn`
  - `res://tests/get_me_test.tscn`
- Or use the minimal showcase: `res://addons/godot_itch/godot_itch_showcase.tscn`.

## Troubleshooting

- OAuth scope limitation: currently, itch.io only supports the `profile:me` scope for OAuth. With OAuth you can call `get_me` and `credentials_info`. itch.io has indicated more scopes may be added in the future.
- “not JWT” error: happens when calling JWT endpoints with an API key. The plugin auto-detects the token type; if making custom requests, use `/key/...` for API keys and `/jwt/...` for OAuth tokens.
- No token available: ensure you launched via the itch app (launcher token) or completed OAuth and saved the token.

## License

MIT — see the repository `LICENSE` file for details.
