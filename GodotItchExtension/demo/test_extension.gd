extends Node

@onready var output: RichTextLabel = $Panel/VBox/MarginContainer/Scroll/Output
@onready var game_id: LineEdit = $Panel/VBox/Buttons/GameId
@onready var download_key: LineEdit = $Panel/VBox/Buttons/DownloadKey

# Simple test to verify the Itch extension loads properly

var error_occurred : bool = false


func _ready():
	game_id.text = ProjectSettings.get_setting("godot_itch/game_id")
	if game_id.text.is_empty():
		game_id.text = "Game is empty. Set value in project settings!"
	print("=== Itch GDExtension Test ===")
	# Test 1: Check if Itch singleton exists
	if Itch:
		print("✓ Itch singleton found")
		print("  Version: ", Itch.get_godotitch_version())
		Itch.api_response.connect(_on_api_response)
		Itch.api_error.connect(_on_api_error)
		Itch.initialize_with_scene(self)
	else:
		print("✗ Itch singleton NOT found")
		return


func _on_download_keys_pressed() -> void:
	var gid = game_id.text.strip_edges()
	output.append_text("[b]Request:[/b] get_game_download_keys - gid: %s, key: %s\n" % [gid, download_key.text])
	Itch.get_game_download_keys(gid, download_key.text)


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

		"get_game_download_keys":
			if data.has("download_key"):
				var download_key_data = data["download_key"]
				print("  Download key verified: ", download_key_data.get("key", "Unknown"))
				print("  Downloads: ", download_key_data.get("downloads", "Unknown"))
				print("  Created at: ", download_key_data.get("created_at", "Unknown"))
			else:
				print("  Download key not found  ")

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


func _on_button_pressed() -> void:
	output.append_text("[b]Testing sequence...[/b]\n")
	Itch.get_my_games()
	await get_tree().create_timer(3.0).timeout
	Itch.get_game_download_keys(game_id.text, download_key.text)
	await get_tree().create_timer(3.0).timeout
	output.append_text("[b]Done.[/b]\n")

func _on_get_me_pressed() -> void:
	output.append_text("[b]Request:[/b] get_me\n")
	Itch.get_me()

func _on_my_games_pressed() -> void:
	output.append_text("[b]Request:[/b] get_my_games\n")
	Itch.get_my_games()

func _on_use_download_key_pressed() -> void:
	var key = download_key.text.strip_edges()
	if key.is_empty():
		output.append_text("[color=yellow]Download key is empty[/color]\n")
		return
	# Placeholder: No direct API call in current code that takes a key; just echo for now.
	output.append_text("[b]Using download key:[/b] %s\n" % key)
