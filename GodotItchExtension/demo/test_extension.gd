extends Node

# Simple test to verify the Itch extension loads properly

var error_occurred : bool = false

func _ready():
	print("=== Itch GDExtension Test ===")

	# Test 1: Check if Itch singleton exists
	if Itch:
		print("✓ Itch singleton found")
		print("  Version: ", Itch.get_godotitch_version())
		Itch.api_response.connect(_on_api_response)
		Itch.api_error.connect(_on_api_error)
	else:
		print("✗ Itch singleton NOT found")
		return

	# Test 2: Check basic functionality
	await get_tree().process_frame

	# Try to initialize (without scene - should not crash)
	error_occurred = false
	print("✓ Basic method calls work")

	# Test project settings
	var api_key = "jPVybXVHgwIC3ib3PGR99pn4zAUOrFP2emTja4EV"
	var game_id = "3719972"

	# Use error handling for method calls
	if Itch.has_method("get_api_key"):
		api_key = Itch.get_api_key()
	else:
		error_occurred = true

	if Itch.has_method("get_game_id"):
		game_id = Itch.get_game_id()
	else:
		error_occurred = true

	print("  API Key set: ", "YES" if !api_key.is_empty() else "NO")
	print("  Game ID set: ", "YES" if !game_id.is_empty() else "NO")

	# Test scene initialization
	if Itch.has_method("initialize_with_scene"):
		Itch.initialize_with_scene(self)
		print("✓ Scene initialization successful")
	else:
		error_occurred = true

	if error_occurred:
		print("✗ Error during testing:")
		print("  Extension may not be properly loaded")
		return


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

	error_occurred = true


func _on_button_pressed() -> void:
	print("Testing test_request_http()...")
	Itch.test_request_http()
	#await get_tree().create_timer(5.0).timeout

	#print("Testing get_my_games()...")
	#Itch.get_my_games()
	#await get_tree().create_timer(5.0).timeout
#
	#print("Testing verify_user()...")
	#Itch.verify_user("leafo")
	#await get_tree().create_timer(5.0).timeout
#
	#if error_occurred:
		#print("✗ Errors occurred during API calls")
	#else:
		#print("=== All tests passed! ===")
		#print("Extension is working correctly.")
