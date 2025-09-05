## Production-ready verification manager for GodotItch
##
## This autoload provides a simplified interface for game developers
## to implement download key verification using project settings.
##
## Usage:
##   # Check if user needs verification
##   if GodotItchManager.needs_verification():
##       GodotItchManager.show_verification_ui()
##   else:
##       start_game()
##
extends Node

## Emitted when verification is completed successfully
signal verification_completed(result: ItchVerificationResult)

## Emitted when verification fails
signal verification_failed(error: String)

## Emitted when verification status changes
signal verification_status_changed(is_verified: bool)

## Current verification client
var _verification_client: ItchVerificationClient

## Cached verification result
var _cached_result: ItchVerificationResult


func _ready() -> void:
	# Validate configuration on startup
	var config_errors := GodotItchConfig.validate_config()
	if not config_errors.is_empty():
		print("[GodotItchManager] Configuration warnings:")
		for error in config_errors:
			print("  - " + error)


## Checks if verification is needed based on project settings
##
## @return: bool indicating if verification is required
func needs_verification() -> bool:
	if not GodotItchConfig.get_require_verification():
		return false
	
	if not GodotItchConfig.is_configured():
		print("[GodotItchManager] Plugin not configured - verification disabled")
		return false
	
	# TODO: Check cached verification in Phase 4
	return true


## Starts the verification process with a download key
##
## @param download_key: The download key to verify (URL or raw key)
func verify_download_key(download_key: String) -> void:
	if not GodotItchConfig.is_configured():
		verification_failed.emit("Plugin not configured in project settings")
		return
	
	# Extract key from URL if needed
	var extracted_key := ItchDownloadKey.extract_from_url(download_key)
	if extracted_key.is_empty():
		var error := ItchDownloadKey.get_validation_error(download_key)
		verification_failed.emit("Invalid download key: " + error)
		return
	
	# Create verification client if needed
	if not _verification_client:
		_verification_client = ItchVerificationClient.new()
		add_child(_verification_client)
		_verification_client.verification_completed.connect(_on_verification_completed)
		_verification_client.verification_failed.connect(_on_verification_failed)
	
	print("[GodotItchManager] Starting verification...")
	
	# Start verification with project settings
	_verification_client.verify_download_key(
		extracted_key,
		GodotItchConfig.get_api_key(),
		GodotItchConfig.get_game_id()
	)


## Gets the current verification status
##
## @return: bool indicating if user is currently verified
func is_verified() -> bool:
	return _cached_result != null and _cached_result.verified


## Gets the cached verification result
##
## @return: ItchVerificationResult or null if not verified
func get_verification_result() -> ItchVerificationResult:
	return _cached_result


## Clears the current verification status
func clear_verification() -> void:
	_cached_result = null
	verification_status_changed.emit(false)
	print("[GodotItchManager] Verification cleared")


## Gets configuration summary for debugging
##
## @return: Dictionary containing configuration and status info
func get_status_summary() -> Dictionary:
	var config := GodotItchConfig.get_config_summary()
	config["is_verified"] = is_verified()
	config["needs_verification"] = needs_verification()
	
	if _cached_result:
		config["verified_user"] = _cached_result.get_user_display_name()
		config["verified_at"] = _cached_result.verified_at
	
	return config


## Handles successful verification
func _on_verification_completed(result: ItchVerificationResult) -> void:
	_cached_result = result
	verification_status_changed.emit(true)
	verification_completed.emit(result)
	
	print("[GodotItchManager] Verification successful: %s" % result.get_user_display_name())


## Handles verification failure
func _on_verification_failed(error: String) -> void:
	_cached_result = null
	verification_status_changed.emit(false)
	verification_failed.emit(error)
	
	print("[GodotItchManager] Verification failed: %s" % error)
