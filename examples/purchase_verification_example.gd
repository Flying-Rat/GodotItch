# Itch Plugin - Purchase Verification Example
# 
# This example demonstrates how to implement purchase verification
# in a game using the GodotItch plugin.
extends Control

@onready var key_input: LineEdit = $VBox/KeyInput
@onready var api_key_input: LineEdit = $VBox/APIKeyInput
@onready var game_id_input: LineEdit = $VBox/GameIDInput
@onready var verify_button: Button = $VBox/VerifyButton
@onready var status_label: Label = $VBox/StatusLabel
@onready var result_area: RichTextLabel = $VBox/ResultArea

func _ready() -> void:
	verify_button.pressed.connect(_on_verify_pressed)
	_connect_to_plugin()
	
	# Load configuration from project settings
	var api_key: String = ProjectSettings.get_setting("godot_itch/api_key", "")
	var game_id: String = ProjectSettings.get_setting("godot_itch/game_id", "")

	if not api_key.is_empty():
		api_key_input.text = api_key
	
	if not game_id.is_empty():
		game_id_input.text = game_id

func _connect_to_plugin() -> void:
	# Connect using the GodotItch class - cleaner and more robust
	GodotItch.connect_verification_started(_on_verification_started)
	GodotItch.connect_verification_completed(_on_verification_completed)
	GodotItch.connect_verification_failed(_on_verification_failed)
	
	# Check current verification status
	var status = GodotItch.get_verification_status()
	if status.verified:
		result_area.text = "[color=green]User already verified![/color]\n"
		result_area.text += "User: %s\n" % status.user_info.get("display_name", "Unknown")
		result_area.text += "Verified at: %s\n" % status.user_info.get("verified_at", "Unknown")
		status_label.text = "Verified"
	else:
		var plugin_info = GodotItch.get_plugin_info()
		if not plugin_info.plugin_enabled:
			status_label.text = "Plugin not available"
			result_area.text = "[color=red]GodotItch plugin not found.[/color]\nPlease ensure the plugin is enabled and restart Godot."

func _on_verify_pressed() -> void:
	var download_key := key_input.text.strip_edges()
	var api_key := api_key_input.text.strip_edges()
	var game_id := game_id_input.text.strip_edges()
	
	# Check plugin availability using GodotItch class
	var plugin_info = GodotItch.get_plugin_info()
	if not plugin_info.plugin_enabled:
		status_label.text = "Error: Plugin not available"
		result_area.text = "[color=red]Error:[/color]\nGodotItch plugin is not available"
		return
	
	# Update project settings if they've been changed
	if not api_key.is_empty() and api_key != ProjectSettings.get_setting("godot_itch/api_key", ""):
		ProjectSettings.set_setting("godot_itch/api_key", api_key)
		ProjectSettings.save()
	
	if not game_id.is_empty() and game_id != ProjectSettings.get_setting("godot_itch/game_id", ""):
		ProjectSettings.set_setting("godot_itch/game_id", game_id)
		ProjectSettings.save()
	
	# Clear previous results
	result_area.text = ""
	status_label.text = "Validating input..."
	
	# Validate download key input using GodotItch class
	var validation_result = GodotItch.validate(download_key)
	
	# Show validation results
	result_area.text = "[color=blue]Input Validation:[/color]\n"
	result_area.text += "Valid: %s\n" % validation_result.valid
	result_area.text += "Extracted Key: %s\n" % validation_result.key
	
	if validation_result.has("info"):
		var info = validation_result.info
		result_area.text += "Input Type: %s\n" % info.get("input_type", "unknown")
		result_area.text += "Key Length: %s\n" % info.get("key_length", "unknown")
	
	if not validation_result.valid:
		status_label.text = "Validation failed"
		result_area.text += "\n[color=red]Validation Error:[/color]\n" + validation_result.error
		return
	
	# Check if configuration is complete
	if api_key.is_empty():
		status_label.text = "Error: API key required"
		result_area.text += "\n[color=red]Configuration Error:[/color]\nAPI key is required for verification."
		return
	
	if game_id.is_empty():
		status_label.text = "Error: Game ID required"
		result_area.text += "\n[color=red]Configuration Error:[/color]\nGame ID is required for verification."
		return
	
	result_area.text += "\n[color=yellow]Starting verification with itch.io...[/color]\n"
	verify_button.disabled = true
	GodotItch.verify(download_key)

func _on_verification_started() -> void:
	status_label.text = "Verifying with itch.io API..."

func _on_verification_completed(user_info: Dictionary) -> void:
	status_label.text = "Purchase verification successful!"
	verify_button.disabled = false
	
	result_area.text += "\n[color=green]✓ Purchase Verified Successfully![/color]\n\n"
	result_area.text += "[b]Customer Information:[/b]\n"
	result_area.text += "User ID: %s\n" % user_info.get("user_id", "N/A")
	result_area.text += "Display Name: %s\n" % user_info.get("display_name", "N/A")
	result_area.text += "Username: %s\n" % user_info.get("username", "N/A")
	result_area.text += "Game: %s\n" % user_info.get("game_title", "N/A")
	result_area.text += "Verified: %s\n" % user_info.get("verified_at", "N/A")

func _on_verification_failed(error_message: String, error_code: String) -> void:
	status_label.text = "Purchase verification failed"
	verify_button.disabled = false
	
	result_area.text += "\n[color=red]✗ Purchase Verification Failed[/color]\n\n"
	result_area.text += "[b]Error Details:[/b]\n"
	result_area.text += "Message: %s\n" % error_message
	result_area.text += "Code: %s\n" % error_code
