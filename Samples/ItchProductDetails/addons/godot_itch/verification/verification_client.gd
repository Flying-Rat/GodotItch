## Itch.io download key verification client
##
## This class handles direct verification of download keys with the itch.io API.
## It provides an async interface using signals for verification results.
##
## Signals:
## - verification_completed(result: ItchVerificationResult): Emitted when verification succeeds
## - verification_failed(error: String): Emitted when verification encounters an error
##
## @tutorial: https://itch.io/docs/api/overview
extends HTTPRequest
class_name ItchVerificationClient


## Emitted when download key verification completes successfully
## @param result: ItchVerificationResult containing verification details
signal verification_completed(result: ItchVerificationResult)

## Emitted when verification fails due to network, API, or authentication errors
## @param error: String describing the error that occurred
signal verification_failed(error: String)

## Base URL for itch.io API
const API_BASE_URL := "https://itch.io/api/1"

## Timeout for verification requests (seconds)
var request_timeout: float = 10.0

## Current verification request data for tracking
var _current_request: Dictionary = {}

## Flag to enable debug logging
var debug_logging: bool = false


func _ready() -> void:
	request_completed.connect(_on_verification_request_completed)


## Verifies a download key using the itch.io API
##
## This method makes a direct API call to itch.io to verify that the download key
## is valid and associated with the specified game.
##
## @param download_key: The download key to verify
## @param api_key: Your itch.io API key (from your itch.io account settings)
## @param game_id: The numeric ID of your game on itch.io
func verify_download_key(download_key: String, api_key: String, game_id: String) -> void:
	if download_key.is_empty():
		_emit_verification_failed("Download key cannot be empty")
		return
	
	if api_key.is_empty():
		_emit_verification_failed("API key cannot be empty")
		return
	
	if game_id.is_empty():
		_emit_verification_failed("Game ID cannot be empty")
		return
	
	# Validate download key format
	if not ItchDownloadKey.validate_format(download_key):
		var error := ItchDownloadKey.get_validation_error(download_key)
		_emit_verification_failed("Invalid download key format: " + error)
		return
	
	if debug_logging:
		print("[ItchVerificationClient] Starting verification for key: %s..." % download_key.substr(0, 8))
	
	# Store request info for response handling
	_current_request = {
		"download_key": download_key,
		"api_key": api_key,
		"game_id": game_id,
		"started_at": Time.get_datetime_string_from_system(false, true)
	}
	
	var request_data := _build_api_request(download_key, api_key, game_id)
	var error := _make_api_request(request_data)
	
	if error != OK:
		_emit_verification_failed("Failed to start verification request: " + _get_error_string(error))


## Builds the API request configuration for itch.io verification
##
## @param key: The download key to verify
## @param api_key: The itch.io API key
## @param game_id: The game ID
## @return: Dictionary containing request configuration
func _build_api_request(key: String, api_key: String, game_id: String) -> Dictionary:
	var url := "%s/key/game/%s/download_keys?download_key=%s" % [API_BASE_URL, game_id, key]
	var headers := PackedStringArray([
		"Authorization: Bearer " + api_key
	])
	
	return {
		"url": url,
		"headers": headers,
		"method": HTTPClient.METHOD_GET
	}


## Makes the actual HTTP request to itch.io API
##
## @param request_data: Dictionary containing request configuration
## @return: Error code from the HTTP request
func _make_api_request(request_data: Dictionary) -> Error:
	if debug_logging:
		print("[ItchVerificationClient] Making API request to: %s" % request_data["url"])
	
	# Set timeout
	if request_timeout > 0:
		set_timeout(request_timeout)
	
	# Make GET request to itch.io API
	var error := request(
		request_data["url"],
		request_data["headers"],
		request_data["method"]
	)
	
	return error


## Handles the completed verification request
##
## @param result: HTTPRequest result code
## @param response_code: HTTP status code
## @param headers: Response headers
## @param body: Response body
func _on_verification_request_completed(
	result: int,
	response_code: int,
	headers: PackedStringArray,
	body: PackedByteArray
) -> void:
	
	if debug_logging:
		print("[ItchVerificationClient] Request completed - Result: %d, Response: %d" % [result, response_code])
	
	# Check for network/request errors
	if result != HTTPRequest.RESULT_SUCCESS:
		_emit_verification_failed("Network error: " + _get_result_string(result))
		return
	
	# Parse response body
	var response_text := ""
	if body.size() > 0:
		response_text = body.get_string_from_utf8()
	
	if debug_logging:
		print("[ItchVerificationClient] Response body: %s" % response_text)
	
	# Parse JSON response
	var json := JSON.new()
	var parse_result := json.parse(response_text)
	
	if parse_result != OK:
		_emit_verification_failed("Invalid JSON response from itch.io API")
		return
	
	var response_data := json.data as Dictionary
	
	# Handle API response based on status code
	match response_code:
		200:
			_handle_successful_verification(response_data)
		401:
			_emit_verification_failed("Invalid API key or unauthorized access")
		404:
			_emit_verification_failed("Game not found or download key invalid")
		429:
			_emit_verification_failed("Rate limit exceeded. Please try again later.")
		_:
			var error_msg := "API error (HTTP %d)" % response_code
			if response_data.has("errors"):
				error_msg += ": " + str(response_data["errors"])
			_emit_verification_failed(error_msg)


## Handles a successful verification response from itch.io
##
## @param response_data: Parsed JSON response from itch.io API
func _handle_successful_verification(response_data: Dictionary) -> void:
	if debug_logging:
		print("[ItchVerificationClient] Processing successful verification response")
	
	# Check if response contains download_key data
	if not response_data.has("download_key"):
		_emit_verification_failed("Invalid API response: missing download_key data")
		return
	
	var download_key_data := response_data["download_key"] as Dictionary
	
	# Extract user information
	var user_info := {}
	if download_key_data.has("owner"):
		user_info = download_key_data["owner"]
	
	# Extract game information if available
	var game_info := {}
	if download_key_data.has("game"):
		game_info = download_key_data["game"]
	
	# Create successful verification result
	var result := ItchVerificationResult.create_success(
		_current_request["download_key"],
		user_info,
		game_info
	)
	
	result.response_code = 200
	result.raw_response = response_data
	
	if debug_logging:
		print("[ItchVerificationClient] Verification successful for user: %s" % result.get_user_display_name())
	
	verification_completed.emit(result)


## Emits verification failed signal with error message
##
## @param error_message: The error message to emit
func _emit_verification_failed(error_message: String) -> void:
	if debug_logging:
		print("[ItchVerificationClient] Verification failed: %s" % error_message)
	
	verification_failed.emit(error_message)


## Converts HTTPRequest result code to human-readable string
##
## @param result_code: The HTTPRequest result code
## @return: String description of the result code
func _get_result_string(result_code: int) -> String:
	match result_code:
		HTTPRequest.RESULT_CHUNKED_BODY_SIZE_MISMATCH:
			return "Chunked body size mismatch"
		HTTPRequest.RESULT_CANT_CONNECT:
			return "Cannot connect to server"
		HTTPRequest.RESULT_CANT_RESOLVE:
			return "Cannot resolve hostname"
		HTTPRequest.RESULT_CONNECTION_ERROR:
			return "Connection error"
		HTTPRequest.RESULT_TLS_HANDSHAKE_ERROR:
			return "TLS handshake error"
		HTTPRequest.RESULT_NO_RESPONSE:
			return "No response from server"
		HTTPRequest.RESULT_BODY_SIZE_LIMIT_EXCEEDED:
			return "Response body too large"
		HTTPRequest.RESULT_BODY_DECOMPRESS_FAILED:
			return "Failed to decompress response"
		HTTPRequest.RESULT_REQUEST_FAILED:
			return "Request failed"
		HTTPRequest.RESULT_DOWNLOAD_FILE_CANT_OPEN:
			return "Cannot open download file"
		HTTPRequest.RESULT_DOWNLOAD_FILE_WRITE_ERROR:
			return "Download file write error"
		HTTPRequest.RESULT_REDIRECT_LIMIT_REACHED:
			return "Too many redirects"
		HTTPRequest.RESULT_TIMEOUT:
			return "Request timeout"
		_:
			return "Unknown error (%d)" % result_code


## Converts Error enum to human-readable string
##
## @param error_code: The Error code
## @return: String description of the error
func _get_error_string(error_code: Error) -> String:
	match error_code:
		ERR_INVALID_PARAMETER:
			return "Invalid parameter"
		ERR_CANT_CONNECT:
			return "Cannot connect"
		ERR_CANT_RESOLVE:
			return "Cannot resolve hostname"
		ERR_UNAUTHORIZED:
			return "Unauthorized"
		ERR_PARAMETER_RANGE_ERROR:
			return "Parameter out of range"
		ERR_OUT_OF_MEMORY:
			return "Out of memory"
		ERR_FILE_NOT_FOUND:
			return "File not found"
		ERR_FILE_BAD_DRIVE:
			return "Bad drive"
		ERR_FILE_BAD_PATH:
			return "Bad path"
		ERR_FILE_NO_PERMISSION:
			return "No permission"
		ERR_FILE_ALREADY_IN_USE:
			return "File already in use"
		ERR_FILE_CANT_OPEN:
			return "Cannot open file"
		ERR_FILE_CANT_WRITE:
			return "Cannot write file"
		ERR_FILE_CANT_READ:
			return "Cannot read file"
		ERR_FILE_UNRECOGNIZED:
			return "Unrecognized file"
		ERR_FILE_CORRUPT:
			return "File corrupt"
		ERR_FILE_MISSING_DEPENDENCIES:
			return "Missing dependencies"
		ERR_FILE_EOF:
			return "End of file"
		ERR_CANT_OPEN:
			return "Cannot open"
		ERR_CANT_CREATE:
			return "Cannot create"
		_:
			return "Error %d" % error_code
