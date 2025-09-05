## GodotItch Plugin - Main Autoload Facade
##
## This is the primary interface for itch.io integration in Godot games.
## Provides a simple API for download key verification with minimal setup.
##
## Usage:
##   # Connect to signals
##   Itch.verification_completed.connect(_on_verified)
##   Itch.verification_failed.connect(_on_failed)
##   
##   # Verify a download key
##   Itch.verify("user_download_key_or_url")
##
## Requirements:
##   - Set godot_itch/api_key in project settings
##   - Set godot_itch/game_id in project settings
##
extends Node

## Emitted when verification process starts
signal verification_started()

## Emitted when verification completes successfully
## @param user_info: Dictionary containing verified user information
signal verification_completed(user_info: Dictionary)

## Emitted when verification fails
## @param error_message: Technical error description
## @param error_code: HTTP status code or error identifier
signal verification_failed(error_message: String, error_code: String)

## Enable detailed logging for debugging
var debug_mode: bool = false

## Current verification status
var _verification_status: Dictionary = {
	"verified": false,
	"user_info": {},
	"error": "",
	"timestamp": 0
}

## Verification client for API calls
var _verification_client: Node


func _ready() -> void:
	# Load debug setting from project settings
	debug_mode = ProjectSettings.get_setting("godot_itch/debug_logging", false)
	
	if debug_mode:
		print("[Itch] Autoload initialized - Debug mode enabled")
	
	# Validate configuration on startup
	_validate_configuration()


## Verify a download key or URL
##
## This method handles the complete verification flow:
## 1. Validates input format
## 2. Checks project configuration  
## 3. Performs network verification
## 4. Emits appropriate signals
##
## @param download_input: Download key or itch.io URL to verify
func verify(download_input: String) -> void:
	if debug_mode:
		print("[Itch] Starting verification for input: %s" % download_input)
	
	# Clear previous status
	_verification_status = {
		"verified": false,
		"user_info": {},
		"error": "",
		"timestamp": Time.get_unix_time_from_system()
	}
	
	# Validate input 
	var validation_result = validate(download_input)
	if not validation_result.valid:
		_handle_verification_failed(validation_result.error, "INVALID_INPUT")
		return
	
	# Check configuration
	var config_error = _check_configuration()
	if not config_error.is_empty():
		_handle_verification_failed(config_error, "CONFIG_ERROR")
		return
	
	# Start verification process
	verification_started.emit()
	
	# Create verification client if needed
	if not _verification_client:
		var verification_client_script = load("res://addons/godot_itch/verification/verification_client.gd")
		_verification_client = verification_client_script.new()
		add_child(_verification_client)
		_verification_client.verification_completed.connect(_on_verification_completed)
		_verification_client.verification_failed.connect(_on_verification_failed)
	
	# Get project settings
	var api_key = ProjectSettings.get_setting("godot_itch/api_key", "")
	var game_id = ProjectSettings.get_setting("godot_itch/game_id", "")
	
	# Start real verification
	_verification_client.verify_download_key(validation_result.key, api_key, game_id)


## Handle successful verification from verification client
func _on_verification_completed(result: ItchVerificationResult) -> void:
	if debug_mode:
		print("[Itch] Verification completed successfully")
	
	# Convert ItchVerificationResult to dictionary format for the signal
	var user_info = {
		"user_id": result.get_user_id(),
		"display_name": result.get_user_display_name(),
		"username": result.user_info.get("username", ""),
		"game_title": result.get_game_title(),
		"verified_at": result.verified_at
	}
	
	_verification_status = {
		"verified": true,
		"user_info": user_info,
		"error": "",
		"timestamp": Time.get_unix_time_from_system()
	}
	
	verification_completed.emit(user_info)


## Handle verification failure from verification client
func _on_verification_failed(error_message: String) -> void:
	_handle_verification_failed(error_message, "API_ERROR")


## Validate download input without performing network verification
##
## @param download_input: Download key or URL to validate
## @return: Dictionary with validation results
## Format: { valid: bool, key: String, error: String, info: Dictionary }
func validate(download_input: String) -> Dictionary:
	var result = {
		"valid": false,
		"key": "",
		"error": "",
		"info": {}
	}
	
	if download_input.is_empty():
		result.error = "Input is empty"
		return result
	
	var input = download_input.strip_edges()
	var extracted_key = _extract_key_from_input(input)
	
	if extracted_key.is_empty():
		result.error = _get_validation_error(input)
		return result
	
	result.valid = true
	result.key = extracted_key
	
	# Determine input type for info
	if "itch.io" in input:
		result.info["input_type"] = "url"
	else:
		result.info["input_type"] = "raw_key"
	
	result.info["key_length"] = extracted_key.length()
	
	return result


## Get current verification status
##
## @return: Dictionary containing verification state and user info
## Format: { verified: bool, user_info: Dictionary, error: String, timestamp: int }
func get_verification_status() -> Dictionary:
	return _verification_status.duplicate()


## Clear current verification status
##
## Useful for logout functionality or account switching
func clear_verification() -> void:
	_verification_status = {
		"verified": false,
		"user_info": {},
		"error": "",
		"timestamp": 0
	}
	
	if debug_mode:
		print("[Itch] Verification status cleared")


## Check if verification is required based on project settings
##
## @return: bool indicating if verification is required
func is_verification_required() -> bool:
	return ProjectSettings.get_setting("godot_itch/require_verification", true)


## Internal: Extract key from various input formats
func _extract_key_from_input(input: String) -> String:
	if input.is_empty():
		return ""
	
	# Check if it looks like an itch.io URL
	if "itch.io" in input:
		# Handle /download/KEY format (most common)
		if "/download/" in input:
			var parts = input.split("/download/")
			if parts.size() > 1:
				var key_part = parts[1].split("?")[0]  # Remove any query parameters
				key_part = key_part.split("/")[0]      # Remove any additional path segments
				if _is_valid_key_format(key_part):
					return key_part
		
		# Handle key= format (alternative)
		if "key=" in input:
			var parts = input.split("key=")
			if parts.size() > 1:
				var key_part = parts[1].split("&")[0]  # Get part before next parameter
				if _is_valid_key_format(key_part):
					return key_part
	
	# Treat as raw key - validate format
	var clean_input = input.strip_edges()
	if _is_valid_key_format(clean_input):
		return clean_input
	
	return ""


## Internal: Check if a string matches valid download key format
func _is_valid_key_format(key: String) -> bool:
	if key.length() < 20 or key.length() > 64:
		return false
	
	# Basic alphanumeric check (including underscores)
	var regex = RegEx.new()
	regex.compile("^[a-zA-Z0-9_]+$")
	return regex.search(key) != null


## Internal: Get validation error for input
func _get_validation_error(input: String) -> String:
	if input.is_empty():
		return "Download key is empty"
	
	# Try to extract key first for better error messages
	if "itch.io" in input:
		if "/download/" in input or "key=" in input:
			return "Could not extract valid download key from URL"
		else:
			return "URL does not appear to be a valid itch.io download link"
	
	# Direct key validation
	if input.length() < 20:
		return "Download key too short (minimum 20 characters)"
	
	if input.length() > 64:
		return "Download key too long (maximum 64 characters)"
	
	return "Invalid download key format (must be alphanumeric with underscores)"


## Internal: Validate project configuration
func _validate_configuration() -> void:
	var config_error = _check_configuration()
	if not config_error.is_empty():
		if debug_mode:
			print("[Itch] Configuration warning: %s" % config_error)


## Internal: Check if required project settings are configured
##
## @return: String with error message, empty if valid
func _check_configuration() -> String:
	var api_key = ProjectSettings.get_setting("godot_itch/api_key", "")
	var game_id = ProjectSettings.get_setting("godot_itch/game_id", "")
	
	if api_key.is_empty():
		return "API key not set in project settings (godot_itch/api_key)"
	
	if game_id.is_empty():
		return "Game ID not set in project settings (godot_itch/game_id)"
	
	return ""


## Internal: Common failure handling
func _handle_verification_failed(error_message: String, error_code: String) -> void:
	if debug_mode:
		print("[Itch] Verification failed: %s (%s)" % [error_message, error_code])
	
	# Update status
	_verification_status.verified = false
	_verification_status.error = error_message
	_verification_status.timestamp = Time.get_unix_time_from_system()
	
	# Emit failure signal
	verification_failed.emit(error_message, error_code)
