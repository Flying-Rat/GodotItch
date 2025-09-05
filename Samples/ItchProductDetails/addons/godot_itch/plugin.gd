@tool
extends EditorPlugin

var _autoload_added := false

# Project settings keys
const SETTING_API_KEY := "godot_itch/api_key"
const SETTING_GAME_ID := "godot_itch/game_id"
const SETTING_REQUIRE_VERIFICATION := "godot_itch/require_verification"
const SETTING_CACHE_DURATION_DAYS := "godot_itch/cache_duration_days"

func _enter_tree() -> void:
	# Add project settings for GodotItch plugin
	_add_project_settings()
	
	# Register the ItchAPI as an autoload singleton so game code can access it easily
	if not ProjectSettings.has_setting("autoload/ItchAPI"):
		add_autoload_singleton("ItchAPI", "res://addons/godot_itch/core/itch_api.gd")
		_autoload_added = true

func _exit_tree() -> void:
	# Remove autoload when plugin is disabled to clean up properly
	if _autoload_added and ProjectSettings.has_setting("autoload/ItchAPI"):
		remove_autoload_singleton("ItchAPI")
		_autoload_added = false
	
	# Note: We don't remove project settings when plugin is disabled
	# as users might want to keep their configuration

## Adds GodotItch project settings to the project settings dialog
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
	
	# Require verification setting
	if not ProjectSettings.has_setting(SETTING_REQUIRE_VERIFICATION):
		ProjectSettings.set_setting(SETTING_REQUIRE_VERIFICATION, true)
		var require_verification_info := {
			"name": SETTING_REQUIRE_VERIFICATION,
			"type": TYPE_BOOL,
			"hint": PROPERTY_HINT_NONE,
			"hint_string": ""
		}
		ProjectSettings.add_property_info(require_verification_info)
	
	# Cache duration setting
	if not ProjectSettings.has_setting(SETTING_CACHE_DURATION_DAYS):
		ProjectSettings.set_setting(SETTING_CACHE_DURATION_DAYS, 7)
		var cache_duration_info := {
			"name": SETTING_CACHE_DURATION_DAYS,
			"type": TYPE_INT,
			"hint": PROPERTY_HINT_RANGE,
			"hint_string": "1,30,1"
		}
		ProjectSettings.add_property_info(cache_duration_info)
	
	# Save the project settings
	ProjectSettings.save()

## Gets the API key from project settings
static func get_api_key() -> String:
	return ProjectSettings.get_setting(SETTING_API_KEY, "")

## Gets the game ID from project settings
static func get_game_id() -> String:
	return ProjectSettings.get_setting(SETTING_GAME_ID, "")

## Gets whether verification is required from project settings
static func get_require_verification() -> bool:
	return ProjectSettings.get_setting(SETTING_REQUIRE_VERIFICATION, true)

## Gets the cache duration in days from project settings
static func get_cache_duration_days() -> int:
	return ProjectSettings.get_setting(SETTING_CACHE_DURATION_DAYS, 7)
