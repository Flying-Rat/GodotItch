## Download key utilities for itch.io integration
##
## This class provides static methods for working with itch.io download keys,
## including validation, extraction from URLs, and key information parsing.
##
## @tutorial: https://itch.io/docs/api/overview
class_name ItchDownloadKey


## Pattern for valid itch.io download keys (alphanumeric + underscore, typically 40-64 characters)
const DOWNLOAD_KEY_PATTERN := "^[a-zA-Z0-9_]{20,64}$"

## Common itch.io download URL patterns
const DOWNLOAD_URL_PATTERNS := [
	# Standard download URL with key
	"itch\\.io/.*[?&]key=([a-fA-F0-9]{20,64})",
	# Alternative download URL format
	"itch\\.io/.*download_key=([a-fA-F0-9]{20,64})",
	# Direct key URL format
	"itch\\.io/game-download/\\?key=([a-fA-F0-9]{20,64})",
	# Path-based download format (e.g., /download/KEY)
	"itch\\.io/.*/download/([a-zA-Z0-9_]{20,64})",
	# Generic path ending with key
	"itch\\.io/.*[/]([a-zA-Z0-9_]{20,64})$"
]


## Extracts download key from various URL formats or validates direct key input
##
## Supports multiple input formats:
## - Direct key: "abc123def456..." (validates and returns if valid)
## - Download URL: "https://username.itch.io/game/download/12345?key=abc123def456..."
## - Any URL containing key parameter
##
## @param url: String that might contain a download key or be a key itself
## @return: String containing the extracted key, or empty string if not found/invalid
static func extract_from_url(url: String) -> String:
	if url.is_empty():
		return ""
	
	var trimmed_url := url.strip_edges()
	
	# First, check if the input is already a valid key
	if validate_format(trimmed_url):
		return trimmed_url
	
	# Try to extract key from URL using regex patterns
	for pattern in DOWNLOAD_URL_PATTERNS:
		var regex := RegEx.new()
		var compile_error := regex.compile(pattern)
		
		if compile_error != OK:
			push_warning("[ItchDownloadKey] Failed to compile regex pattern: " + pattern)
			continue
		
		var result := regex.search(trimmed_url)
		if result:
			var extracted_key := result.get_string(1)
			if validate_format(extracted_key):
				return extracted_key
	
	# Fallback: look for any alphanumeric string that might be a key
	var fallback_regex := RegEx.new()
	var compile_error := fallback_regex.compile("[?&]key=([a-zA-Z0-9_]+)")
	
	if compile_error == OK:
		var result := fallback_regex.search(trimmed_url)
		if result:
			var potential_key := result.get_string(1)
			if validate_format(potential_key):
				return potential_key
	
	return ""


## Validates if a string matches the expected download key format
##
## Checks if the key:
## - Is between 20-64 characters long
## - Contains only alphanumeric characters and underscores (0-9, a-z, A-Z, _)
##
## @param key: String to validate as download key
## @return: bool indicating if the key format is valid
static func validate_format(key: String) -> bool:
	if key.is_empty():
		return false
	
	var regex := RegEx.new()
	var compile_error := regex.compile(DOWNLOAD_KEY_PATTERN)
	
	if compile_error != OK:
		push_error("[ItchDownloadKey] Failed to compile validation regex")
		return false
	
	return regex.search(key) != null


## Normalizes a download key by converting to lowercase and trimming whitespace
##
## @param key: The key to normalize
## @return: String containing the normalized key
static func normalize_key(key: String) -> String:
	return key.strip_edges().to_lower()


## Extracts information about a download key without validating it with itch.io
##
## Returns basic information that can be determined from the key format:
## - "length": int - Length of the key
## - "format": String - "alphanumeric" if valid format, "invalid" otherwise
## - "normalized": String - Normalized version of the key
##
## @param key: The download key to analyze
## @return: Dictionary containing key information
static func get_key_info(key: String) -> Dictionary:
	var info := {
		"length": key.length(),
		"format": "invalid",
		"normalized": normalize_key(key),
		"is_valid_format": false
	}
	
	if validate_format(key):
		info["format"] = "alphanumeric"
		info["is_valid_format"] = true
	
	return info


## Generates a user-friendly description of key validation issues
##
## @param key: The key that failed validation
## @return: String describing why the key is invalid
static func get_validation_error(key: String) -> String:
	if key.is_empty():
		return "Download key cannot be empty"
	
	var trimmed := key.strip_edges()
	
	if trimmed.length() < 20:
		return "Download key is too short (minimum 20 characters)"
	
	if trimmed.length() > 64:
		return "Download key is too long (maximum 64 characters)"
	
	# Check for invalid characters
	var regex := RegEx.new()
	var compile_error := regex.compile("[^a-zA-Z0-9_]")
	
	if compile_error == OK:
		var invalid_chars := regex.search(trimmed)
		if invalid_chars:
			return "Download key contains invalid characters (only 0-9, a-z, and _ allowed)"
	
	return "Download key format is invalid"
