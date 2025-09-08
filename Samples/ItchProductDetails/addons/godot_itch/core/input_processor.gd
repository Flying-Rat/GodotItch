## Input Processor for itch.io Download Keys
##
## Handles parsing and validation of download keys from various input formats:
## - Raw download keys
## - Full itch.io download URLs
## - Partial URLs and other formats
##
## This centralizes all input processing logic that was previously scattered
## across example scripts and UI components.
##
class_name ItchInputProcessor

# Preload dependencies
const ItchDownloadKey = preload("res://addons/godot_itch/core/download_key.gd")


## Process and validate download input from user
##
## Handles multiple input formats and extracts the actual download key.
## Returns validation results without performing network operations.
##
## @param raw_input: User input (download key, URL, etc.)
## @return: Dictionary with processing results
## Format: { valid: bool, key: String, error: String, info: Dictionary }
static func process_input(raw_input: String) -> Dictionary:
	var result = {
		"valid": false,
		"key": "",
		"error": "",
		"info": {}
	}
	
	if raw_input.is_empty():
		result.error = "Input is empty"
		return result
	
	var input = raw_input.strip_edges()
	
	# Try to extract key from URL first
	var extracted_key = extract_key_from_url(input)
	if not extracted_key.is_empty():
		result.key = extracted_key
		result.info["input_type"] = "url"
		result.info["original_input"] = input
	else:
		# Treat as raw key
		result.key = input
		result.info["input_type"] = "raw_key"
	
	# Validate the extracted/raw key
	var validation_error = validate_key_format(result.key)
	if validation_error.is_empty():
		result.valid = true
		result.info["key_length"] = result.key.length()
		result.info["key_format"] = _detect_key_format(result.key)
	else:
		result.error = validation_error
	
	return result


## Extract download key from itch.io URL
##
## Supports various URL formats from itch.io download pages
##
## @param url: Full or partial URL containing download key
## @return: Extracted key or empty string if not found
static func extract_key_from_url(url: String) -> String:
	# Use existing ItchDownloadKey logic for URL extraction
	return ItchDownloadKey.extract_from_url(url)


## Validate download key format
##
## Checks if the key meets basic format requirements
##
## @param key: Raw download key to validate
## @return: Error message or empty string if valid
static func validate_key_format(key: String) -> String:
	if key.is_empty():
		return "Download key is empty"
	
	# Use existing ItchDownloadKey validation
	return ItchDownloadKey.get_validation_error(key)


## Get detailed information about a download key
##
## @param key: Download key to analyze
## @return: Dictionary with key information
static func get_key_info(key: String) -> Dictionary:
	# Use existing ItchDownloadKey info function
	return ItchDownloadKey.get_key_info(key)


## Internal: Detect key format type
##
## @param key: Download key to analyze
## @return: String describing the key format
static func _detect_key_format(key: String) -> String:
	var info = get_key_info(key)
	return info.get("format", "unknown")


## Get validation summary for debugging
##
## @param raw_input: Original user input
## @return: Formatted string with validation details
static func get_validation_summary(raw_input: String) -> String:
	var result = process_input(raw_input)
	var summary = []
	
	summary.append("Input: '%s'" % raw_input)
	summary.append("Valid: %s" % result.valid)
	summary.append("Extracted Key: '%s'" % result.key)
	
	if not result.error.is_empty():
		summary.append("Error: %s" % result.error)
	
	if result.info.has("input_type"):
		summary.append("Input Type: %s" % result.info.input_type)
	
	if result.info.has("key_format"):
		summary.append("Key Format: %s" % result.info.key_format)
	
	return "\n".join(summary)
