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
	
	# Initialize the Itch plugin with this scene as context
	Itch.initialize_with_scene(self)

	# Connect to plugin signals
	if Itch:
		Itch.verify_purchase_result.connect(_on_verify_purchase_result)		

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
		Itch.verify_purchase(download_key)
	else:
		result_label.text = "Error: GodotItch plugin not available"
		verify_button.disabled = false

func _on_verify_purchase_result(verified: bool, data: Dictionary) -> void:
	if verified:
		print("Verification succeeded for user: " + str(data))
	else:		
		print("Verification failed with data: " + str(data))

	result_label.text = "Verified: " + str(verified)
	verify_button.disabled = false
