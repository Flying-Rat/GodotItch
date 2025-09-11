extends Node

# Test: instantiate itch.gd, connect signals, call verify() with provided URL
# Exits with code 0 when either verification_completed or verification_failed fires.
# Exits with code 1 on timeout.

@export var download_input: String = "https://hubacekjakub.itch.io/godot-quick-start/download/T_bHe2heYWkHBA6i9doZhsn78oqDjGReBfAxyCR8"
@export var download_input_key: String = "T_bHe2heYWkHBA6i9doZhsn78oqDjGReBfAxyCR8"

var itch: Node
var timeout_timer: Timer

func _ready():
	print("[TEST] Starting verify_download_key_test")

	# Load secrets from file
	var file = FileAccess.open("res://tests/verify_download_key_test.secrets", FileAccess.READ)
	if file:
		var json = JSON.new()
		var error = json.parse(file.get_as_text())
		if error == OK:
			var data = json.get_data()
			ProjectSettings.set_setting("godot_itch/api_key", data["api_key"])
			ProjectSettings.set_setting("godot_itch/game_id", data["game_id"])
		else:
			print("Error parsing secrets JSON")
	else:
		print("Error opening secrets file")

	# Enable plugin debug logging for more verbose output during the test
	if ProjectSettings.has_setting("godot_itch/debug_logging"):
		ProjectSettings.set_setting("godot_itch/debug_logging", true)

	print("[SIMPLE TEST] starting")

	# TODO(jakub.hubacek): find a way around this
	Itch.initialize_with_scene(self)
	# Connect signal handlers (use top-level methods, not local lambdas)
	Itch.verify_purchase_result.connect(_on_verify_purchase)

	print("[SIMPLE TEST] verifying:", download_input_key)
	# TODO(jakub.huabcek): allow option to paste whole url with key
	Itch.verify_purchase(download_input_key)

	# Quit after 20s if nothing happens
	get_tree().create_timer(40.0).timeout.connect(self._on_timeout)

func _on_verify_purchase(verified: bool, _data: Dictionary) -> void:
	if verified:
		print("[SIMPLE TEST] User is verified:")
	else:		
		print("[SIMPLE TEST] User is not verified:")

func _on_timeout() -> void:
	print("[SIMPLE TEST] timeout")
