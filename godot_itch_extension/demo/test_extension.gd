extends Node

@onready var output: RichTextLabel = $Panel/VBox/MarginContainer/Scroll/Output
@onready var download_key_line_edit: LineEdit = $Panel/VBox/Buttons/DownloadKey

# Simple test to verify the Itch extension loads properly

var error_occurred : bool = false
var download_key : String = ""
var game_id : String
var api_key : String


func _ready():
	print("=== Itch GDExtension Test ===")

	# Test 1: Check if Itch singleton exists
	if Itch:
		print("✓ Itch singleton found")
		print("  Version: ", Itch.get_godotitch_version())
		Itch.api_response.connect(_on_api_response)
		Itch.api_error.connect(_on_api_error)
		Itch.verify_purchase_result.connect(_on_verify_purchase)
	else:
		print("✗ Itch singleton NOT found")
		return

	# Test 2: Check basic functionality
	await get_tree().process_frame

	# Try to initialize (without scene - should not crash)
	error_occurred = false
	print("✓ Basic method calls work")

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
	var text = "[b]API Response:[/b] %s\n%s\n" % [endpoint, JSON.stringify(data, "  ")]
	output.append_text(text)

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

		"get_download_key":
			if data.has("download_key"):
				var download_key_data = data["download_key"]
				print("  Download key: ", download_key_data.get("key", "Unknown"))
			else:
				print("  Download key not found or verification failed")

		"get_game_purchases":
			if data.has("purchases"):
				var purchases = data["purchases"]
				print("  Found ", purchases.size(), " purchases")

		"get_game_uploads":
			if data.has("uploads"):
				var uploads = data["uploads"]
				print("  Found ", uploads.size(), " uploads")

func _on_api_error(endpoint: String, error_message: String, response_code: int):
	var text = "[color=red][b]API Error:[/b] %s (%d) - %s[/color]\n" % [endpoint, response_code, error_message]
	output.append_text(text)

	if response_code == 401:
		print("  This usually means your API key is invalid or missing")
	elif response_code == 404:
		print("  This usually means the resource (user/game) was not found")
	elif response_code == 403:
		print("  This usually means you don't have permission to access this resource")

	error_occurred = true

func _on_verify_purchase(verified: bool, data: Dictionary) -> void:
	if verified:
		output.append_text("[color=green]Download key is VALID![/color]")
	else:		
		output.append_text("[color=red]Download key is INVALID![/color]")

func _on_button_pressed() -> void:
	output.append_text("[b]Testing sequence...[/b]\n")
	Itch.get_my_games()
	await get_tree().create_timer(3.0).timeout
	Itch.get_download_key(download_key, game_id)
	await get_tree().create_timer(3.0).timeout
	output.append_text("[b]Done.[/b]\n")

func _on_get_me_pressed() -> void:
	output.append_text("[b]Request:[/b] get_me\n")
	Itch.get_me()

func _on_my_games_pressed() -> void:
	output.append_text("[b]Request:[/b] get_my_games\n")
	Itch.get_my_games()


func _on_btn_download_key_pressed() -> void:
	var download_key_text = download_key_line_edit.text.strip_edges()
	if download_key_text.is_empty():
		output.append_text("[c]Download key is empty![c]")
		return
	download_key = download_key_text;
	output.append_text("[b]Request:[/b] get_download_key %s\n" % download_key)
	Itch.get_download_key(download_key, game_id)

func _on_btn_verify_purchase_pressed() -> void:
	var download_key_text = download_key_line_edit.text.strip_edges()
	if download_key_text.is_empty():
		output.append_text("[color=red]Download key is empty![/color]")
		return
	download_key = download_key_text
	output.append_text("[b]Request:[/b] verify_purchase %s\n" % download_key)
	Itch.verify_purchase(download_key)
