# GodotItch Plugin - Purchase Verification Example
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
	
	# Connect to the GodotItch plugin
	var itch = get_node_or_null("/root/Itch")
	if itch:
		itch.verification_started.connect(_on_verification_started)
		itch.verification_completed.connect(_on_verification_completed)
		itch.verification_failed.connect(_on_verification_failed)
		print("[VerificationExample] Connected to GodotItch plugin")
	else:
		print("[VerificationExample] ERROR: GodotItch plugin not found. Please enable the plugin in Project Settings.")
		status_label.text = "Plugin not enabled"
		result_area.text = "[color=red]GodotItch plugin not found.[/color]\nPlease enable the plugin in Project Settings > Plugins."
		return
	
	# Load configuration from project settings
	var api_key: String = ProjectSettings.get_setting("godot_itch/api_key", "")
	var game_id: String = ProjectSettings.get_setting("godot_itch/game_id", "")
	
	if not api_key.is_empty():
		api_key_input.text = api_key
	
	if not game_id.is_empty():
		game_id_input.text = game_id
	
	# Check current verification status
	if itch:
		var status = itch.get_verification_status()
		if status.verified:
			result_area.text = "[color=green]User already verified![/color]\n"
			result_area.text += "User: %s\n" % status.user_info.get("display_name", "Unknown")
			result_area.text += "Verified at: %s\n" % status.user_info.get("verified_at", "Unknown")
			status_label.text = "Verified"

func _on_verify_pressed() -> void:
	var download_key := key_input.text.strip_edges()
	var api_key := api_key_input.text.strip_edges()
	var game_id := game_id_input.text.strip_edges()
	
	# Get the GodotItch plugin
	var itch = get_node_or_null("/root/Itch")
	if not itch:
		status_label.text = "Error: Plugin not enabled"
		result_area.text = "[color=red]Error:[/color]\nGodotItch plugin is not enabled or not working"
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
	
	# Validate download key input
	var validation_result = itch.validate(download_key)
	
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
		result_area.text += "\n[color=red]Configuration Error:[/color]\nAPI key is required for verification.\nPlease set it in Project Settings > godot_itch/api_key"
		return
	
	if game_id.is_empty():
		status_label.text = "Error: Game ID required"
		result_area.text += "\n[color=red]Configuration Error:[/color]\nGame ID is required for verification.\nPlease set it in Project Settings > godot_itch/game_id"
		return
	
	result_area.text += "\n[color=yellow]Starting verification with itch.io...[/color]\n"
	verify_button.disabled = true
	
	# Start verification using the plugin
	itch.verify(download_key)

func _on_verification_started() -> void:
	status_label.text = "Verifying with itch.io API..."
	result_area.text += "[color=yellow]Contacting itch.io servers...[/color]\n"

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
	
	result_area.text += "\n[color=green]The user has a valid purchase and can access the game.[/color]"

func _on_verification_failed(error_message: String, error_code: String) -> void:
	status_label.text = "Purchase verification failed"
	verify_button.disabled = false
	
	result_area.text += "\n[color=red]✗ Purchase Verification Failed[/color]\n\n"
	result_area.text += "[b]Error Details:[/b]\n"
	result_area.text += "Message: %s\n" % error_message
	result_area.text += "Code: %s\n" % error_code
	
	# Provide helpful guidance based on error type
	if error_code == "INVALID_INPUT":
		result_area.text += "\n[color=yellow]Please check that you entered a valid download key or itch.io URL.[/color]"
	elif error_code == "CONFIG_ERROR":
		result_area.text += "\n[color=yellow]Please check your API key and Game ID in the project settings.[/color]"
	elif error_code == "API_ERROR":
		result_area.text += "\n[color=yellow]This could indicate an invalid download key, network issue, or API problem.[/color]"
	
	result_area.text += "\n[color=orange]The user cannot access premium content until verification succeeds.[/color]"
