@tool
extends EditorPlugin

var _autoload_added := false

# Project settings keys
const SETTING_API_KEY := "godot_itch/api_key"
const SETTING_GAME_ID := "godot_itch/game_id"
const SETTING_DEBUG_LOGGING := "godot_itch/advanced/debug_logging"

# UI elements for the Itch panel
var itch_panel
var api_key_edit
var game_id_edit
var debug_logging_check

func _enter_tree() -> void:
	_add_project_settings()
	_create_itch_project_settings_panel()
	add_tool_menu_item("Itch Settings", Callable(self, "_on_itch_settings_menu"))

	#if not ProjectSettings.has_setting("autoload/Itch"):
		#add_autoload_singleton("Itch", "res://addons/godot_itch/autoload/itch.gd")
		#_autoload_added = true

func _exit_tree() -> void:
	#if _autoload_added and ProjectSettings.has_setting("autoload/Itch"):
		#remove_autoload_singleton("Itch")
		#_autoload_added = false

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
			"usage": PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_EDITOR_BASIC_SETTING ,
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
			"usage": PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_EDITOR_BASIC_SETTING,
			"hint_string": "Your game's numeric ID from itch.io"
		}
		ProjectSettings.add_property_info(game_id_info)
	
	# Debug logging setting
	if not ProjectSettings.has_setting(SETTING_DEBUG_LOGGING):
		ProjectSettings.set_setting(SETTING_DEBUG_LOGGING, false)
		var debug_logging_info := {
			"name": SETTING_DEBUG_LOGGING,
			"type": TYPE_BOOL,
			"hint": PROPERTY_HINT_NONE,
			"usage": PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_EDITOR_BASIC_SETTING,
			"hint_string": ""
		}
		ProjectSettings.add_property_info(debug_logging_info)
	
	# Save the project settings
	ProjectSettings.save()

## Creates a panel for Itch settings and adds it to the Project Settings container
func _create_itch_project_settings_panel() -> void:
	# Load a styled settings scene and use it as the panel
	var scene = load("res://addons/godot_itch/settings/itch_settings_panel.tscn")
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
		
		add_control_to_container(CustomControlContainer.CONTAINER_PROJECT_SETTING_TAB_RIGHT, itch_panel)

## Removes the Itch panel from the Project Settings container
func _remove_itch_project_settings_panel() -> void:
	if itch_panel:
		remove_control_from_container(CustomControlContainer.CONTAINER_PROJECT_SETTING_TAB_RIGHT, itch_panel)
		itch_panel.queue_free()
