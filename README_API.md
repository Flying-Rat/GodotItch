# Itch.io API GDExtension

This GDExtension provides access to the itch.io serverside API for Godot 4 projects.

## Features

- **User Management**: Get current user info, verify users
- **Game Management**: Get your games, purchases, and uploads  
- **Async API**: All requests are asynchronous with signal-based responses
- **Project Settings Integration**: Store API keys and game IDs in project settings
- **Error Handling**: Comprehensive error reporting with HTTP status codes

## Setup

1. **Get your API key** from https://itch.io/user/settings/api-keys
2. **Set up the extension** in your project
3. **Configure your API key** using one of these methods:
   - Via code: `Itch.set_api_key("your_api_key_here")`
   - Via project settings: Set `godot_itch/api_key`

## Basic Usage

```gdscript
extends Node

func _ready():
    # Initialize HTTPRequest with current scene
    Itch.initialize_with_scene(self)
    
    # Connect to API signals
    Itch.connect("api_response", _on_api_response)
    Itch.connect("api_error", _on_api_error)
    
    # Set API key
    Itch.set_api_key("your_api_key_here")
    
    # Make API calls
    Itch.get_me()

func _on_api_response(endpoint: String, data: Dictionary):
    print("Success: ", endpoint, " - ", data)

func _on_api_error(endpoint: String, error: String, code: int):
    print("Error: ", endpoint, " - ", error, " (", code, ")")
```

## Available Methods

### User API
- `get_me()` - Get current user information
- `verify_user(username: String)` - Verify if a user exists

### Games API  
- `get_my_games()` - Get all your games
- `get_game_purchases(game_id: String = "")` - Get purchases for a game
- `get_game_uploads(game_id: String = "")` - Get uploads for a game

### Configuration
- `set_api_key(api_key: String)` - Set API key
- `set_game_id(game_id: String)` - Set default game ID
- `get_api_key()` - Get current API key
- `get_game_id()` - Get current game ID
- `initialize_with_scene(scene: Node)` - Initialize HTTPRequest

## Signals

### api_response(endpoint: String, data: Dictionary)
Emitted when an API call succeeds.
- `endpoint`: The API method called (e.g., "get_me", "get_my_games")
- `data`: Response data from itch.io API

### api_error(endpoint: String, error_message: String, response_code: int)
Emitted when an API call fails.
- `endpoint`: The API method that failed
- `error_message`: Description of the error
- `response_code`: HTTP status code

## API Response Examples

### get_me()
```json
{
  "user": {
    "id": 123456,
    "username": "myusername",
    "display_name": "My Display Name",
    "url": "https://myusername.itch.io",
    "cover_url": "https://...",
    "gamer": true,
    "developer": true
  }
}
```

### get_my_games()
```json
{
  "games": [
    {
      "id": 123456,
      "title": "My Awesome Game",
      "url": "https://myusername.itch.io/my-awesome-game",
      "short_text": "A great game",
      "type": "default",
      "classification": "game",
      "published": true
    }
  ]
}
```

### verify_user(username)
```json
{
  "user": {
    "id": 123456,
    "username": "targetuser",
    "display_name": "Target User"
  }
}
```

## Error Codes

- `401` - Unauthorized (invalid API key)
- `403` - Forbidden (insufficient permissions)
- `404` - Not Found (user/game doesn't exist)
- `429` - Rate Limited (too many requests)
- `500` - Server Error (itch.io server issue)

## Project Settings

The extension automatically creates these project settings:
- `godot_itch/api_key` - Your itch.io API key
- `godot_itch/game_id` - Default game ID for game-specific calls

## Autoload Setup

The extension automatically registers `Itch` as a singleton, so you can access it from anywhere in your project:

```gdscript
# Access from any script
Itch.get_me()
```

## Security Notes

- **Never commit your API key** to version control
- **Use project settings** or environment variables for API keys
- **Validate responses** before using data in production
- **Handle rate limiting** gracefully in your application

## Troubleshooting

1. **"HTTPRequest not initialized"**: Call `Itch.initialize_with_scene(self)` first
2. **401 Unauthorized**: Check your API key is correct
3. **No response**: Ensure you're connected to the internet
4. **JSON parse error**: The itch.io API might be returning non-JSON (check error message)

## Links

- [itch.io API Documentation](https://itch.io/docs/api/serverside)
- [itch.io API Keys](https://itch.io/user/settings/api-keys)
- [GitHub Repository](https://github.com/Flying-Rat/GodotItch)
