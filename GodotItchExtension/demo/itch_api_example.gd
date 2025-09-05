extends Node2D

# Example script showing how to use the Itch GDExtension API

func _ready():
	# Wait a frame to ensure the singleton is ready
	await get_tree().process_frame
	
	# Initialize the HTTPRequest with current scene
	Itch.initialize_with_scene(self)
	
	# Connect to the API signals
	Itch.connect("api_response", _on_api_response)
	Itch.connect("api_error", _on_api_error)
	
	# Set your API key (you can also set this in project settings)
	# Get your API key from: https://itch.io/user/settings/api-keys
	# Itch.set_api_key("your_api_key_here")
	
	# Set your game ID (optional, needed for game-specific calls)
	# Itch.set_game_id("123456")
	
	print("Itch API initialized. Version: ", Itch.get_godotitch_version())
	
	# Example API calls (uncomment to test):
	# test_api_calls()

func test_api_calls():
	print("Testing itch.io API calls...")
	
	# Get current user info
	Itch.get_me()
	
	# Get user's games
	await get_tree().create_timer(1.0).timeout
	Itch.get_my_games()
	
	# Verify a specific user
	await get_tree().create_timer(1.0).timeout
	Itch.verify_user("someusername")
	
	# Get purchases for your game (requires game_id to be set)
	# await get_tree().create_timer(1.0).timeout
	# Itch.get_game_purchases()
	
	# Get uploads for your game (requires game_id to be set)
	# await get_tree().create_timer(1.0).timeout
	# Itch.get_game_uploads()

func _on_api_response(endpoint: String, data: Dictionary):
	print("API Response for ", endpoint, ":")
	print("  Data: ", data)
	
	match endpoint:
		"get_me":
			if data.has("user"):
				var user = data["user"]
				print("  User: ", user.get("username", "Unknown"))
				print("  Display Name: ", user.get("display_name", "N/A"))
		
		"get_my_games":
			if data.has("games"):
				var games = data["games"]
				print("  Found ", games.size(), " games")
				for game in games:
					print("    - ", game.get("title", "Unknown Game"))
		
		"verify_user":
			if data.has("user"):
				var user = data["user"]
				print("  User verified: ", user.get("username", "Unknown"))
			else:
				print("  User not found or verification failed")
		
		"get_game_purchases":
			if data.has("purchases"):
				var purchases = data["purchases"]
				print("  Found ", purchases.size(), " purchases")
		
		"get_game_uploads":
			if data.has("uploads"):
				var uploads = data["uploads"]
				print("  Found ", uploads.size(), " uploads")

func _on_api_error(endpoint: String, error_message: String, response_code: int):
	print("API Error for ", endpoint, ":")
	print("  Error: ", error_message)
	print("  Response Code: ", response_code)
	
	if response_code == 401:
		print("  This usually means your API key is invalid or missing")
	elif response_code == 404:
		print("  This usually means the resource (user/game) was not found")
	elif response_code == 403:
		print("  This usually means you don't have permission to access this resource")

func _input(event):
	if event.is_pressed():
		match event.keycode:
			KEY_1:
				print("Testing get_me()...")
				Itch.get_me()
			KEY_2:
				print("Testing get_my_games()...")
				Itch.get_my_games()
			KEY_3:
				print("Testing verify_user()...")
				Itch.verify_user("leafo")  # itch.io creator's username
			KEY_H:
				print("Itch API Help:")
				print("1 - Get current user info")
				print("2 - Get my games")
				print("3 - Verify user 'leafo'")
				print("H - Show this help")
				print("\nMake sure to set your API key first!")
				print("API Key: ", "SET" if !Itch.get_api_key().is_empty() else "NOT SET")
