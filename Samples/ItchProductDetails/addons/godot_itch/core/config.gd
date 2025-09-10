## GodotItch plugin configuration helper
##
## This class provides easy access to Itch plugin settings
## configured in the project settings.
##
## Usage:
##   var api_key = ItchConfig.get_api_key()
##   var game_id = ItchConfig.get_game_id()
##
class_name ItchConfig


# Project settings keys
const SETTING_API_KEY := "godot_itch/api_key"
const SETTING_GAME_ID := "godot_itch/game_id"
const SETTING_REQUIRE_VERIFICATION := "godot_itch/require_verification"
const SETTING_CACHE_DURATION_DAYS := "godot_itch/cache_duration_days"


## Gets the API key from project settings
##
## @return: String containing the itch.io API key, or empty string if not set
static func get_api_key() -> String:
	return ProjectSettings.get_setting(SETTING_API_KEY, "")


## Gets the game ID from project settings
##
## @return: String containing the game ID, or empty string if not set
static func get_game_id() -> String:
	return ProjectSettings.get_setting(SETTING_GAME_ID, "")


## Gets whether verification is required from project settings
##
## @return: bool indicating if verification is mandatory (default: true)
static func get_require_verification() -> bool:
	return ProjectSettings.get_setting(SETTING_REQUIRE_VERIFICATION, true)


## Gets the cache duration in days from project settings
##
## @return: int number of days to cache verification results (default: 7)
static func get_cache_duration_days() -> int:
	return ProjectSettings.get_setting(SETTING_CACHE_DURATION_DAYS, 7)


## Checks if the plugin is properly configured
##
## @return: bool indicating if both API key and game ID are set
static func is_configured() -> bool:
	var api_key := get_api_key()
	var game_id := get_game_id()
	return not api_key.is_empty() and not game_id.is_empty()


## Gets a configuration summary for debugging
##
## @return: Dictionary containing configuration status
static func get_config_summary() -> Dictionary:
	var api_key := get_api_key()
	return {
		"api_key_set": not api_key.is_empty(),
		"api_key_preview": api_key.substr(0, 8) + "..." if not api_key.is_empty() else "",
		"game_id": get_game_id(),
		"require_verification": get_require_verification(),
		"cache_duration_days": get_cache_duration_days(),
		"is_configured": is_configured()
	}


## Validates the current configuration
##
## @return: Array of error messages, empty if configuration is valid
static func validate_config() -> Array[String]:
	var errors: Array[String] = []
	
	var api_key := get_api_key()
	if api_key.is_empty():
		errors.append("API key is not set in project settings")
	elif api_key.length() < 10:
		errors.append("API key appears to be too short")
	
	var game_id := get_game_id()
	if game_id.is_empty():
		errors.append("Game ID is not set in project settings")
	elif not game_id.is_valid_int():
		errors.append("Game ID must be a numeric value")
	
	var cache_days := get_cache_duration_days()
	if cache_days < 1 or cache_days > 30:
		errors.append("Cache duration must be between 1 and 30 days")
	
	return errors
