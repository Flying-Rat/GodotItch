## Itch - Simple Plugin Interface
##
## This provides a clean, simple way to access Itch functionality
## without requiring users to manage autoloads or complex setup.
##
## Usage:
##   # Connect to signals
##   Itch.connect_verification_completed(_on_verified)
##   Itch.connect_verification_failed(_on_failed)
##   
##   # Verify a download key  
##   Itch.verify("user_download_key_or_url")
##
## Requirements:
##   - Enable Itch plugin in Project Settings
##   - Set godot_itch/api_key in project settings
##   - Set godot_itch/game_id in project settings
##
class_name Itch


## Access the internal Itch autoload singleton
static func _get_itch_singleton():
	# Access the autoload directly through the scene tree
	var main_loop = Engine.get_main_loop()
	if main_loop is SceneTree:
		var scene_tree = main_loop as SceneTree
		return scene_tree.root.get_node_or_null("Itch")
	return null


## Verify a download key or URL
##
## @param download_input: Download key or itch.io URL to verify
static func verify(download_input: String) -> void:
	var itch = _get_itch_singleton()
	if itch and itch.has_method("verify"):
		itch.verify(download_input)
	else:
	push_error("[Itch] Plugin not properly initialized. Make sure the Itch plugin is enabled.")


## Validate download input without performing network verification
##
## @param download_input: Download key or URL to validate
## @return: Dictionary with validation results
static func validate(download_input: String) -> Dictionary:
	var itch = _get_itch_singleton()
	if itch and itch.has_method("validate"):
		return itch.validate(download_input)
	else:
	push_error("[Itch] Plugin not properly initialized. Make sure the Itch plugin is enabled.")
		return {"valid": false, "error": "Plugin not initialized", "key": "", "info": {}}


## Get current verification status
##
## @return: Dictionary containing verification state and user info
static func get_verification_status() -> Dictionary:
	var itch = _get_itch_singleton()
	if itch and itch.has_method("get_verification_status"):
		return itch.get_verification_status()
	else:
		return {"verified": false, "user_info": {}, "error": "Plugin not initialized", "timestamp": 0}


## Clear current verification status
static func clear_verification() -> void:
	var itch = _get_itch_singleton()
	if itch and itch.has_method("clear_verification"):
		itch.clear_verification()


## Check if verification is required based on project settings
##
## @return: bool indicating if verification is required
static func is_verification_required() -> bool:
	var itch = _get_itch_singleton()
	if itch and itch.has_method("is_verification_required"):
		return itch.is_verification_required()
	else:
		return ProjectSettings.get_setting("godot_itch/require_verification", true)


## Connect to verification started signal
##
## @param callable: Function to call when verification starts
static func connect_verification_started(callable: Callable) -> void:
	var itch = _get_itch_singleton()
	if itch and itch.has_signal("verification_started"):
		if not itch.verification_started.is_connected(callable):
			itch.verification_started.connect(callable)
	else:
	push_warning("[Itch] Cannot connect to verification_started signal - plugin not available")


## Connect to verification completed signal
##
## @param callable: Function to call when verification completes successfully
static func connect_verification_completed(callable: Callable) -> void:
	var itch = _get_itch_singleton()
	if itch and itch.has_signal("verification_completed"):
		if not itch.verification_completed.is_connected(callable):
			itch.verification_completed.connect(callable)
	else:
	push_warning("[Itch] Cannot connect to verification_completed signal - plugin not available")


## Connect to verification failed signal
##
## @param callable: Function to call when verification fails
static func connect_verification_failed(callable: Callable) -> void:
	var itch = _get_itch_singleton()
	if itch and itch.has_signal("verification_failed"):
		if not itch.verification_failed.is_connected(callable):
			itch.verification_failed.connect(callable)
	else:
	push_warning("[Itch] Cannot connect to verification_failed signal - plugin not available")


## Disconnect from verification started signal
##
## @param callable: Function to disconnect
static func disconnect_verification_started(callable: Callable) -> void:
	var itch = _get_itch_singleton()
	if itch and itch.has_signal("verification_started"):
		if itch.verification_started.is_connected(callable):
			itch.verification_started.disconnect(callable)


## Disconnect from verification completed signal
##
## @param callable: Function to disconnect
static func disconnect_verification_completed(callable: Callable) -> void:
	var itch = _get_itch_singleton()
	if itch and itch.has_signal("verification_completed"):
		if itch.verification_completed.is_connected(callable):
			itch.verification_completed.disconnect(callable)


## Disconnect from verification failed signal
##
## @param callable: Function to disconnect
static func disconnect_verification_failed(callable: Callable) -> void:
	var itch = _get_itch_singleton()
	if itch and itch.has_signal("verification_failed"):
		if itch.verification_failed.is_connected(callable):
			itch.verification_failed.disconnect(callable)


## Get plugin information
##
## @return: Dictionary with plugin status and configuration
static func get_plugin_info() -> Dictionary:
	var itch = _get_itch_singleton()
	var info = {
		"plugin_enabled": itch != null,
		"version": "1.0.0",
		"autoload_available": itch != null
	}
	
	if itch:
		info["debug_mode"] = itch.debug_mode if itch.has_method("get") else false
		var status = get_verification_status()
		info["verified"] = status.verified
	
	return info
