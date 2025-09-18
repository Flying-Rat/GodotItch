# Itch Plugin

A Godot 4 plugin + GDExtension for interacting with the itch.io API. It supports OAuth (JWT) or launcher API key authentication and exposes common endpoints to your game.

## Features

- ✅ OAuth helper (open browser + save token)
- ✅ Launcher detection (uses `ITCHIO_API_KEY` if present)
- ✅ Credentials info: `get_credentials_info()`
- ✅ User profile: `get_me()`
- ✅ Async requests with signals (`api_response`, `api_error`)
- ✅ Debug logging and in-editor settings panel


## Quick Start

### 1. Installation
1. Copy `addons/godot_itch` to your project's `addons/` directory
2. Enable the plugin in **Project Settings > Plugins**
3. Restart Godot

### 2. Project Settings
Configure under **Project Settings > godot_itch**:
- `game_id`: Your itch.io Game ID
- `oauth_client_id`: OAuth client_id from your itch developer settings
- `oauth_redirect_uri`: Redirect URI (leave blank for out-of-band: `urn:ietf:wg:oauth:2.0:oob`)
- `oauth_scope`: Set to `profile:me` (currently the only supported scope)

### 3. Basic Usage (Get Credentials + Me)
```gdscript
extends Control

func _ready():
    # 1) Initialize HTTPRequest node usage
    Itch.initialize_with_scene(self)

    # 2) Connect response signals
    Itch.api_response.connect(_on_api_response)
    Itch.api_error.connect(_on_api_error)

    # 3) Authenticate in either way:
    #    a) If launched via itch app, ITCHIO_API_KEY will be used automatically.
    #    b) Or start OAuth in browser, then paste & save token:
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

Tips:
- You can persist OAuth values via `ProjectSettings` by using the UI in the included demo.
- The extension auto-selects the correct server endpoint (`/key` for API key, `/jwt` for JWT) and always sends `Authorization: Bearer <token>`.


## API Reference

### Itch (Facade)
- `initialize_with_scene(scene_node: Node)`
  - Adds/initializes the internal `HTTPRequest` so requests can run.
- `get_credentials_info()`
  - Calls `GET /api/1/{key|jwt}/credentials/info`. Emits `api_response("credentials_info", data)`.
- `get_me()`
  - Calls `GET /api/1/{key|jwt}/me`. Emits `api_response("get_me", data)`.

Auth helpers (delegated to `Itch.get_auth()`):
- `set_oauth_client_id(client_id: String)`
- `set_oauth_redirect_uri(redirect_uri: String)`
- `set_oauth_scope(scope: String)` – use `profile:me`
- `build_oauth_authorize_url(client_id: String = "", redirect_uri: String = "", state: String = "") -> String`
- `start_oauth_authorization(client_id: String = "", redirect_uri: String = "", state: String = "")`

### Signals
- `api_response(endpoint: String, data: Dictionary)` – Fired on 2xx responses
- `api_error(endpoint: String, error_message: String, response_code: int)` – Fired on HTTP/parse errors


## Troubleshooting

- OAuth scope limitation: itch.io currently only supports the `profile:me` scope for OAuth flows. That means only `get_me` and `credentials_info` are usable with OAuth at this time. itch.io has indicated they plan to extend scopes in the future.
- “not JWT” error: This occurs if the JWT endpoint is called with an API key. The extension auto-detects and selects the correct endpoint; if you implement custom calls, use `/api/1/key/...` for API keys and `/api/1/jwt/...` for JWT tokens, always with `Authorization: Bearer <token>`.
- No token available: If not launched from the itch app (no `ITCHIO_API_KEY`) and you haven’t performed OAuth, calls will fail. Start OAuth, then paste/save the token via `Itch.get_auth().set_oauth_token(...)`.
- Debug logging: Enable `godot_itch/advanced/debug_logging` for verbose output.


## Testing

Demo/manual tests included:
- `res://tests/credentials_info_test.tscn` – Opens OAuth, saves token, runs `get_credentials_info()`
- `res://tests/get_me_test.tscn` – Opens OAuth, saves token, runs `get_me()`

Open either scene in the editor and press Run. When launched via the itch app, the app’s API key can be used instead of OAuth.


## License
See main project [license](LICENSE) for details.
