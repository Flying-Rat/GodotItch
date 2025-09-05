# Phase 1 Verification Test
extends Control

@onready var key_input: LineEdit = $VBox/KeyInput
@onready var api_key_input: LineEdit = $VBox/APIKeyInput
@onready var game_id_input: LineEdit = $VBox/GameIDInput
@onready var verify_button: Button = $VBox/VerifyButton
@onready var status_label: Label = $VBox/StatusLabel
@onready var result_area: RichTextLabel = $VBox/ResultArea

var verification_client: ItchVerificationClient

func _ready() -> void:
	verify_button.pressed.connect(_on_verify_pressed)
	
	# Create verification client
	verification_client = ItchVerificationClient.new()
	add_child(verification_client)
	
	# Connect signals
	verification_client.verification_completed.connect(_on_verification_completed)
	verification_client.verification_failed.connect(_on_verification_failed)
	
	# Enable debug logging
	verification_client.debug_logging = true

	# Load API key and game ID from project settings if available
	var config_api_key := GodotItchConfig.get_api_key()
	var config_game_id := GodotItchConfig.get_game_id()
	
	if not config_api_key.is_empty():
		api_key_input.text = config_api_key
	else:
		print("[VerificationTest] WARNING: API key not set in project settings")
	
	if not config_game_id.is_empty():
		game_id_input.text = config_game_id
	else:
		print("[VerificationTest] WARNING: Game ID not set in project settings")

func _on_verify_pressed() -> void:
	var download_key := key_input.text.strip_edges()
	var api_key := api_key_input.text.strip_edges()
	var game_id := game_id_input.text.strip_edges()
	
	# Clear previous results
	result_area.text = ""
	status_label.text = "Validating input..."
	
	# Extract key from URL if needed
	var extracted_key := ItchDownloadKey.extract_from_url(download_key)
	if extracted_key.is_empty():
		status_label.text = "Error: Invalid download key format"
		var error := ItchDownloadKey.get_validation_error(download_key)
		result_area.text = "[color=red]Validation Error:[/color]\n" + error
		return
	
	if api_key.is_empty():
		status_label.text = "Error: API key required"
		result_area.text = "[color=red]Error:[/color]\nAPI key is required for verification"
		return
	
	if game_id.is_empty():
		status_label.text = "Error: Game ID required"
		result_area.text = "[color=red]Error:[/color]\nGame ID is required for verification"
		return
	
	# Show key info
	var key_info := ItchDownloadKey.get_key_info(extracted_key)
	result_area.text = "[color=blue]Key Info:[/color]\n"
	result_area.text += "Length: %d characters\n" % key_info["length"]
	result_area.text += "Format: %s\n" % key_info["format"]
	result_area.text += "Valid: %s\n\n" % key_info["is_valid_format"]
	
	status_label.text = "Verifying with itch.io..."
	verify_button.disabled = true
	
	# Start verification
	verification_client.verify_download_key(extracted_key, api_key, game_id)

func _on_verification_completed(result: ItchVerificationResult) -> void:
	status_label.text = "Verification completed successfully!"
	verify_button.disabled = false
	
	result_area.text += "[color=green]Verification Success![/color]\n\n"
	result_area.text += "User: %s\n" % result.get_user_display_name()
	result_area.text += "User ID: %s\n" % result.get_user_id()
	
	if result.has_valid_user_info():
		result_area.text += "Username: %s\n" % result.user_info.get("username", "N/A")
	
	var game_title := result.get_game_title()
	if not game_title.is_empty():
		result_area.text += "Game: %s\n" % game_title
	
	result_area.text += "Verified at: %s\n" % result.verified_at

func _on_verification_failed(error: String) -> void:
	status_label.text = "Verification failed"
	verify_button.disabled = false
	
	result_area.text += "[color=red]Verification Failed![/color]\n\n"
	result_area.text += "Error: %s\n" % error
