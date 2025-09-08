## Data structure for itch.io download key verification results
##
## This resource class contains the result of a download key verification,
## including success status, user information, and error details.
##
## @tutorial: https://itch.io/docs/api/overview
class_name ItchVerificationResult
extends Resource


## Whether the download key was successfully verified
@export var verified: bool = false

## The download key that was verified
@export var download_key: String = ""

## User information returned from itch.io API
## Expected fields: id, username, display_name, cover_url, url
@export var user_info: Dictionary = {}

## Game information if available from verification
## Expected fields: id, title, url, short_text, type
@export var game_info: Dictionary = {}

## Error message if verification failed
@export var error_message: String = ""

## Timestamp when verification was completed (ISO 8601 format)
@export var verified_at: String = ""

## HTTP response code from the verification request
@export var response_code: int = 0

## Raw API response for debugging purposes (optional)
@export var raw_response: Dictionary = {}


## Constructor for creating a new verification result
func _init(
	p_verified: bool = false,
	p_download_key: String = "",
	p_user_info: Dictionary = {},
	p_error_message: String = ""
) -> void:
	verified = p_verified
	download_key = p_download_key
	user_info = p_user_info
	error_message = p_error_message
	verified_at = Time.get_datetime_string_from_system(false, true)


## Creates a successful verification result
##
## @param download_key: The verified download key
## @param user_info: User information from itch.io API
## @param game_info: Optional game information
## @return: ItchVerificationResult indicating success
static func create_success(
	download_key: String,
	user_info: Dictionary,
	game_info: Dictionary = {}
) -> ItchVerificationResult:
	var result := ItchVerificationResult.new()
	result.verified = true
	result.download_key = download_key
	result.user_info = user_info
	result.game_info = game_info
	result.verified_at = Time.get_datetime_string_from_system(false, true)
	result.response_code = 200
	return result


## Creates a failed verification result
##
## @param download_key: The download key that failed verification
## @param error_message: Description of the failure
## @param response_code: HTTP response code if applicable
## @return: ItchVerificationResult indicating failure
static func create_failure(
	download_key: String,
	error_message: String,
	response_code: int = 0
) -> ItchVerificationResult:
	var result := ItchVerificationResult.new()
	result.verified = false
	result.download_key = download_key
	result.error_message = error_message
	result.verified_at = Time.get_datetime_string_from_system(false, true)
	result.response_code = response_code
	return result


## Gets the user's display name or username as fallback
##
## @return: String containing the user's display name
func get_user_display_name() -> String:
	if user_info.has("display_name") and not user_info["display_name"].is_empty():
		return user_info["display_name"]
	elif user_info.has("username"):
		return user_info["username"]
	else:
		return "Unknown User"


## Gets the user's unique ID
##
## @return: String containing the user's ID, or empty string if not available
func get_user_id() -> String:
	if user_info.has("id"):
		return str(user_info["id"])
	return ""


## Gets the game title if available
##
## @return: String containing the game title, or empty string if not available
func get_game_title() -> String:
	if game_info.has("title"):
		return game_info["title"]
	return ""


## Checks if the verification result contains valid user information
##
## @return: bool indicating if user info is present and valid
func has_valid_user_info() -> bool:
	return verified and user_info.has("id") and user_info.has("username")


## Gets a summary of the verification result for display purposes
##
## @return: String containing a human-readable summary
func get_summary() -> String:
	if verified:
		var user_name := get_user_display_name()
		var game_title := get_game_title()
		
		if not game_title.is_empty():
			return "Verified: %s owns %s" % [user_name, game_title]
		else:
			return "Verified: %s" % user_name
	else:
		return "Verification failed: %s" % error_message


## Converts the result to a dictionary for serialization
##
## @return: Dictionary containing all result data
func to_dict() -> Dictionary:
	return {
		"verified": verified,
		"download_key": download_key,
		"user_info": user_info,
		"game_info": game_info,
		"error_message": error_message,
		"verified_at": verified_at,
		"response_code": response_code
	}


## Creates a verification result from a dictionary
##
## @param data: Dictionary containing verification result data
## @return: ItchVerificationResult created from the dictionary
static func from_dict(data: Dictionary) -> ItchVerificationResult:
	var result := ItchVerificationResult.new()
	result.verified = data.get("verified", false)
	result.download_key = data.get("download_key", "")
	result.user_info = data.get("user_info", {})
	result.game_info = data.get("game_info", {})
	result.error_message = data.get("error_message", "")
	result.verified_at = data.get("verified_at", "")
	result.response_code = data.get("response_code", 0)
	return result
