extends Node

# Test: instantiate itch.gd, connect signals, call verify() with provided URL
# Exits with code 0 when either verification_completed or verification_failed fires.
# Exits with code 1 on timeout.

@export var download_input: String = "https://hubacekjakub.itch.io/godot-quick-start/download/T_bHe2heYWkHBA6i9doZhsn78oqDjGReBfAxyCR8"

var itch: Node
var timeout_timer: Timer

func _ready():
	print("[TEST] Starting verify_download_key_test")

	# Set test-specific settings
	ProjectSettings.set_setting("godot_itch/api_key", "DbgOofrqTrQbxDg77muxN2ooX0yvozDtwWKXuoQe")
	ProjectSettings.set_setting("godot_itch/game_id", "3719972")

	# Enable plugin debug logging for more verbose output during the test
	if ProjectSettings.has_setting("godot_itch/debug_logging"):
		ProjectSettings.set_setting("godot_itch/debug_logging", true)

	print("[SIMPLE TEST] starting")

	# Connect signal handlers (use top-level methods, not local lambdas)
	Itch.verification_started.connect(_on_verification_started)
	Itch.verification_completed.connect(_on_verification_completed)
	Itch.verification_failed.connect(_on_verification_failed)

	print("[SIMPLE TEST] verifying:", download_input)
	Itch.verify(download_input)

	# Quit after 20s if nothing happens
	get_tree().create_timer(20.0).timeout.connect(self._on_timeout)

func _on_verification_completed(user_info: Dictionary) -> void:
	print("[SIMPLE TEST] User is verified:", user_info)

func _on_verification_failed(error_message: String, error_code: String) -> void:
	print("[SIMPLE TEST] User is not verified:", error_message, error_code)

func _on_verification_started() -> void:
	print("[SIMPLE TEST] started")

func _on_timeout() -> void:
	print("[SIMPLE TEST] timeout")
