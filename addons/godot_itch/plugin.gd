@tool
extends EditorPlugin

var _autoload_added := false

# Project settings keys
const SETTING_API_KEY := "godot_itch/api_key"
const SETTING_GAME_ID := "godot_itch/game_id"

# UI elements for the Itch panel
var itch_panel
var api_key_edit
var game_id_edit
var debug_logging_check

func _enter_tree() -> void:
	_add_project_settings()
	_create_itch_project_settings_panel()
	add_tool_menu_item("Itch Settings", Callable(self, "_on_itch_settings_menu"))

	if not ProjectSettings.has_setting("autoload/Itch"):
		add_autoload_singleton("Itch", "res://addons/godot_itch/autoload/itch.gd")
		_autoload_added = true

func _exit_tree() -> void:
	if _autoload_added and ProjectSettings.has_setting("autoload/Itch"):
		remove_autoload_singleton("Itch")
		_autoload_added = false

	remove_tool_menu_item("Itch Settings")
	_remove_itch_project_settings_panel()

## Adds Itch project settings to the project settings dialog
func _add_project_settings() -> void:
	# API Key setting
	if not ProjectSettings.has_setting(SETTING_API_KEY):
		ProjectSettings.set_setting(SETTING_API_KEY, "")
		var api_key_info := {
			"name": SETTING_API_KEY,
			"type": TYPE_STRING,
			"hint": PROPERTY_HINT_PLACEHOLDER_TEXT,
			"hint_string": "Your itch.io API key from account settings"
		}
		ProjectSettings.add_property_info(api_key_info)
	
	# Game ID setting
	if not ProjectSettings.has_setting(SETTING_GAME_ID):
		ProjectSettings.set_setting(SETTING_GAME_ID, "")
		var game_id_info := {
			"name": SETTING_GAME_ID,
			"type": TYPE_STRING,
			"hint": PROPERTY_HINT_PLACEHOLDER_TEXT,
			"hint_string": "Your game's numeric ID from itch.io"
		}
		ProjectSettings.add_property_info(game_id_info)
	
	# Debug logging setting
	if not ProjectSettings.has_setting("godot_itch/debug_logging"):
		ProjectSettings.set_setting("godot_itch/debug_logging", false)
		var debug_logging_info := {
			"name": "godot_itch/debug_logging",
			"type": TYPE_BOOL,
			"hint": PROPERTY_HINT_NONE,
			"hint_string": ""
		}
		ProjectSettings.add_property_info(debug_logging_info)
	
	# Save the project settings
	ProjectSettings.save()

## Creates a panel for Itch settings and adds it to the Project Settings container
func _create_itch_project_settings_panel() -> void:
	# Load a styled settings scene and use it as the panel
	var scene = load("res://addons/godot_itch/itch_settings_panel.tscn")
	if scene:
		itch_panel = scene.instantiate()
		# Wire UI elements from the scene using exact paths for better performance
		api_key_edit = itch_panel.get_node("MainMargin/HBoxRoot/RightPanel/RightMargin/VBox/CredentialsSection/CredentialsGrid/APIKey")
		game_id_edit = itch_panel.get_node("MainMargin/HBoxRoot/RightPanel/RightMargin/VBox/CredentialsSection/CredentialsGrid/GameID")
		debug_logging_check = itch_panel.get_node("MainMargin/HBoxRoot/RightPanel/RightMargin/VBox/OptionsSection/OptionsGrid/DebugLogging")
		var save_button = itch_panel.get_node("MainMargin/HBoxRoot/RightPanel/RightMargin/VBox/ButtonsSection/Buttons/SaveButton")
		
		if save_button:
			save_button.connect("pressed", Callable(self, "_on_save_settings"))
		else:
			printerr("Itch plugin: SaveButton not found at expected path")
		
		# Test connection removed from settings UI; testing is tracked in the roadmap

		# Initialize values (guard each node)
		if api_key_edit:
			api_key_edit.text = get_api_key()
		else:
			printerr("Itch plugin: APIKey control missing at expected path")
			_debug_dump_scene_tree(itch_panel)

		if game_id_edit:
			game_id_edit.text = get_game_id()
		else:
			printerr("Itch plugin: GameID control missing at expected path")

		if debug_logging_check:
			debug_logging_check.set_pressed(ProjectSettings.get_setting("godot_itch/debug_logging", false))
		else:
			printerr("Itch plugin: DebugLogging control missing at expected path")
			_debug_dump_scene_tree(itch_panel)

		add_control_to_container(CustomControlContainer.CONTAINER_PROJECT_SETTING_TAB_RIGHT, itch_panel)

## Handles the Itch Settings menu item (focuses the Project Settings dialog)
func _on_itch_settings_menu() -> void:
	# Attempt to open Project Settings so user can see the panel.
	# EditorInterface has no direct open_project_settings method; instead we
	# toggle the Project Settings dialog via the Editor's UI menu if available.
	# As a simple fallback, notify the user in the editor log and hope the panel
	# is visible in the Project Settings dialog.
	print("Open Project > Project Settings and look for the 'Itch' panel/tab on the right side.")

## Removes the Itch panel from the Project Settings container
func _remove_itch_project_settings_panel() -> void:
	if itch_panel:
		remove_control_from_container(CustomControlContainer.CONTAINER_PROJECT_SETTING_TAB_RIGHT, itch_panel)
		itch_panel.queue_free()

## Saves the settings from the panel to ProjectSettings
func _on_save_settings() -> void:
	ProjectSettings.set_setting(SETTING_API_KEY, api_key_edit.text)
	ProjectSettings.set_setting(SETTING_GAME_ID, game_id_edit.text)
	ProjectSettings.set_setting("godot_itch/debug_logging", debug_logging_check.is_pressed())
	ProjectSettings.save()
	print("Itch plugin: Settings saved successfully!")

## Tests the API connection with current settings
func _on_test_connection() -> void:
	var api_key = api_key_edit.text.strip_edges()
	var game_id = game_id_edit.text.strip_edges()
	
	if api_key.is_empty():
		printerr("Itch plugin: API Key is required for testing connection")
		return
		
	if game_id.is_empty():
		printerr("Itch plugin: Game ID is required for testing connection")
		return
	
	print("Itch plugin: Testing connection to itch.io API...")
	print("Itch plugin: API Key: " + api_key.substr(0, 8) + "..." + " (length: " + str(api_key.length()) + ")")
	print("Itch plugin: Game ID: " + game_id)
	
	# Create a simple HTTP request to test the API
	var http_request = HTTPRequest.new()
	add_child(http_request)
	http_request.connect("request_completed", Callable(self, "_on_test_request_completed"))
	
	var url = "https://itch.io/api/1/key/game-uploads"
	var headers = ["Authorization: Bearer " + api_key]
	var result = http_request.request(url, headers)
	
	if result != OK:
		printerr("Itch plugin: Failed to initiate test request")
		http_request.queue_free()

## Handles the test connection response
func _on_test_request_completed(result: int, response_code: int, headers: PackedStringArray, body: PackedByteArray) -> void:
	var sender = get_children().filter(func(child): return child is HTTPRequest).back()
	if sender:
		sender.queue_free()
	
	if response_code == 200:
		print("Itch plugin: ✅ Connection test successful! API key is valid.")
	elif response_code == 401:
		printerr("Itch plugin: ❌ Connection test failed: Invalid API key")
	elif response_code == 403:
		printerr("Itch plugin: ❌ Connection test failed: API key doesn't have required permissions")
	else:
		printerr("Itch plugin: ❌ Connection test failed: HTTP " + str(response_code))
		if body.size() > 0:
			var response_text = body.get_string_from_utf8()
			print("Response: " + response_text)

## Gets the API key from project settings
static func get_api_key() -> String:
	return ProjectSettings.get_setting(SETTING_API_KEY, "")

## Gets the game ID from project settings
static func get_game_id() -> String:
	return ProjectSettings.get_setting(SETTING_GAME_ID, "")

## Debug helper: dump scene tree to help find missing nodes
func _debug_dump_scene_tree(scene_root) -> void:
	if not scene_root:
		printerr("_debug_dump_scene_tree: scene_root is null")
		return
	printerr("Itch settings scene tree:")

	# Iterative stack-based traversal to avoid nested function/lambda issues
	var stack := []
	stack.append([scene_root, ""])
	while stack.size() > 0:
		var pair = stack.pop_back()
		var node = pair[0]
		var prefix = pair[1]
		printerr("%s- %s (%s)" % [prefix, node.name, node.get_class()])
		var children = node.get_children()
		# push children in reverse order so first child is printed first
		for i in range(children.size() - 1, -1, -1):
			stack.append([children[i], prefix + "  "])
