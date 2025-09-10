## Simple GodotItch Plugin Demo
##
## Basic usage example with minimal setup
extends Control

@onready var api_key_input: LineEdit = $VBox/ApiKeyInput
@onready var game_id_input: LineEdit = $VBox/GameIdInput
@onready var download_key_input: LineEdit = $VBox/DownloadKeyInput
@onready var verify_button: Button = $VBox/VerifyButton
@onready var result_label: Label = $VBox/ResultLabel

func _ready() -> void:
	verify_button.pressed.connect(_on_verify_pressed)
	
	# Connect to plugin signals
	if Itch:
		Itch.verification_completed.connect(_on_verification_completed)
		Itch.verification_failed.connect(_on_verification_failed)
		Itch.verification_started.connect(_on_verification_started)
		

func _on_verify_pressed() -> void:
	var api_key = api_key_input.text.strip_edges()
	var game_id = game_id_input.text.strip_edges()
	var download_key = download_key_input.text.strip_edges()
	
	# Basic validation
	if api_key.is_empty():
		result_label.text = "Error: API Key is required"
		return
	if game_id.is_empty():
		result_label.text = "Error: Game ID is required"
		return
	if download_key.is_empty():
		result_label.text = "Error: Download Key is required"
		return
	
	# Set project settings
	ProjectSettings.set_setting("godot_itch/api_key", api_key)
	ProjectSettings.set_setting("godot_itch/game_id", game_id)
	
	# Start verification
	result_label.text = "Verifying..."
	verify_button.disabled = true
	
	if Itch:
		Itch.verify(download_key)
	else:
		result_label.text = "Error: GodotItch plugin not available"
		verify_button.disabled = false

func _on_verification_completed(user_info: Dictionary) -> void:
	print("Verification succeeded for user: " + str(user_info))
	result_label.text = "Success! User: " + user_info.get("display_name", "Unknown")
	verify_button.disabled = false

func _on_verification_failed(error_message: String, error_code: String) -> void:
	print("Verification failed: " + error_message + " (code: " + error_code + ")")
	result_label.text = "Failed: " + error_message
	verify_button.disabled = false
	
func _on_verification_started() -> void:
	print("Verification started...")
	result_label.text = "Verifying..."
	verify_button.disabled = true
