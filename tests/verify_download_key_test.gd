extends Control

# Test: instantiate itch.gd, connect signals, call verify() with provided URL
# Enhanced with visual UI for manual testing

@export var download_input: String = "https://hubacekjakub.itch.io/godot-quick-start/download/T_bHe2heYWkHBA6i9doZhsn78oqDjGReBfAxyCR8"
@export var download_input_key: String = "T_bHe2heYWkHBA6i9doZhsn78oqDjGReBfAxyCR8"

var itch: Node
var timeout_timer: Timer
var check_timer: Timer

# UI References
@onready var api_key_input: LineEdit = $VBoxContainer/ApiKeyContainer/ApiKeyInput
@onready var game_id_input: LineEdit = $VBoxContainer/GameIdContainer/GameIdInput
@onready var download_key_input: LineEdit = $VBoxContainer/DownloadKeyContainer/DownloadKeyInput
@onready var verify_button: Button = $VBoxContainer/ButtonContainer/VerifyButton
@onready var clear_button: Button = $VBoxContainer/ButtonContainer/ClearButton
@onready var status_label: Label = $VBoxContainer/StatusContainer/StatusLabel
@onready var result_label: Label = $VBoxContainer/StatusContainer/ResultLabel

var secrets_loaded: bool = false

func _ready():
	print("[TEST] Starting verify_download_key_test with visual UI")
	
	# Connect UI signals
	verify_button.pressed.connect(_on_verify_button_pressed)
	clear_button.pressed.connect(_on_clear_button_pressed)
	
	# Try to load secrets from file
	_load_secrets_file()
	
	# Enable plugin debug logging for more verbose output during the test
	if ProjectSettings.has_setting("godot_itch/debug_logging"):
		ProjectSettings.set_setting("godot_itch/debug_logging", true)
	
	# Set default values in input fields if no secrets loaded
	if not secrets_loaded:
		download_key_input.text = download_input_key
		status_label.text = "Status: Enter credentials and download key to test"
	else:
		download_key_input.text = download_input_key
		status_label.text = "Status: Secrets loaded from file, ready to test"
	
	# Initialize Itch system
	_initialize_itch_system()

func _load_secrets_file():
	"""Try to load secrets from the secrets file and populate settings"""
	var file = FileAccess.open("res://tests/verify_download_key_test.secrets", FileAccess.READ)
	if file:
		var json = JSON.new()
		var error = json.parse(file.get_as_text())
		file.close()
		
		if error == OK:
			var data = json.get_data()
			if data.has("api_key") and data.has("game_id"):
				ProjectSettings.set_setting("godot_itch/api_key", data["api_key"])
				ProjectSettings.set_setting("godot_itch/game_id", data["game_id"])
				
				# Populate UI fields with loaded values (masked for security)
				api_key_input.text = "*".repeat(min(data["api_key"].length(), 32))
				game_id_input.text = str(data["game_id"])
				
				secrets_loaded = true
				print("[TEST] Secrets loaded successfully from file")
				return
			else:
				print("[TEST] Error: secrets file missing required fields (api_key, game_id)")
		else:
			print("[TEST] Error parsing secrets JSON:", error)
	else:
		print("[TEST] No secrets file found, will use manual input")

func _initialize_itch_system():
	"""Initialize the Itch system and connect signals"""
	print("[TEST] Initializing Itch system...")
	
	# TODO(jakub.hubacek): find a way around this
	Itch.initialize_with_scene(self)
	# Connect signal handlers
	Itch.verify_purchase_result.connect(_on_verify_purchase)
	
	print("[TEST] Itch system initialized")

func _on_verify_button_pressed():
	"""Handle verify button press"""
	print("[TEST] Verify button pressed")
	status_label.text = "Status: Verifying..."
	result_label.text = ""
	
	# Use input field values if secrets weren't loaded from file
	if not secrets_loaded:
		var api_key = api_key_input.text.strip_edges()
		var game_id = game_id_input.text.strip_edges()
		
		if api_key.is_empty() or game_id.is_empty():
			status_label.text = "Status: Error - API Key and Game ID are required"
			result_label.text = "Please fill in all required fields"
			return
		
		# Update project settings with input values
		ProjectSettings.set_setting("godot_itch/api_key", api_key)
		ProjectSettings.set_setting("godot_itch/game_id", game_id)
		print("[TEST] Using manual input - API Key length:", api_key.length(), ", Game ID:", game_id)
	
	var download_key = download_key_input.text.strip_edges()
	if download_key.is_empty():
		status_label.text = "Status: Error - Download Key is required"
		result_label.text = "Please enter a download key to verify"
		return
	
	print("[TEST] Starting verification for download key:", download_key)
	
	# Debug: Check if Itch is available
	if not Itch:
		print("[TEST] ERROR: Itch system is null!")
		status_label.text = "Status: Error - Itch system not available"
		return
	else:
		print("[TEST] Itch system is available")
	
	# Start verification
	print("[TEST] Calling Itch.verify_download_key...")
	Itch.verify_download_key(download_key)
	print("[TEST] Verification call completed, waiting for response...")
	
	# Set up periodic status checks
	check_timer = Timer.new()
	add_child(check_timer)
	check_timer.wait_time = 2.0
	check_timer.timeout.connect(_on_status_check)
	check_timer.start()
	
	# Set timeout
	get_tree().create_timer(40.0).timeout.connect(_on_timeout)

func _on_clear_button_pressed():
	"""Clear all input fields"""
	if not secrets_loaded:
		api_key_input.text = ""
		game_id_input.text = ""
	download_key_input.text = ""
	status_label.text = "Status: Fields cleared"
	result_label.text = ""

func _on_verify_purchase(verified: bool, data: Dictionary) -> void:
	"""Handle verification result"""
	print("[TEST] _on_verify_purchase callback triggered")
	print("[TEST] Verified:", verified)
	if data:
		print("[TEST] Response data keys:", data.keys())
	else:
		print("[TEST] Response data is null")
	
	# Stop the periodic status check timer
	if check_timer and is_instance_valid(check_timer):
		check_timer.stop()
		check_timer.queue_free()
		check_timer = null
		print("[TEST] Status check timer stopped")
	
	if verified:
		print("[TEST] User is verified successfully")
		status_label.text = "Status: ✅ Verification Successful"
		result_label.text = "Download key is valid and user is entitled to this game"
	else:		
		print("[TEST] User verification failed")
		status_label.text = "Status: ❌ Verification Failed" 
		result_label.text = "Download key is invalid or user is not entitled"
	
	# Log the full response data
	print("[TEST] Full response data:", data)
	
	# Show additional details if available
	if data.has("errors"):
		result_label.text += "\nErrors: " + str(data["errors"])
	elif data.has("download_keys"):
		var keys = data["download_keys"]
		result_label.text += "\nFound " + str(keys.size()) + " matching download keys"
	
	print("[TEST] _on_verify_purchase callback completed")

func _on_status_check() -> void:
	"""Periodic status check during verification"""
	print("[TEST] Status check - still waiting for response...")

func _on_timeout() -> void:
	"""Handle verification timeout"""
	print("[TEST] Verification timeout")
	
	# Stop the periodic status check timer
	if check_timer and is_instance_valid(check_timer):
		check_timer.stop()
		check_timer.queue_free()
		check_timer = null
		print("[TEST] Status check timer stopped due to timeout")
	
	status_label.text = "Status: ⏱️ Verification Timeout"
	result_label.text = "Verification took too long - check network connection and credentials"
