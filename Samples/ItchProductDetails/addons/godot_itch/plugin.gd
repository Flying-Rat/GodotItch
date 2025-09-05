@tool
extends EditorPlugin

var _autoload_added := false

func _enter_tree() -> void:
	# Register the ItchAPI as an autoload singleton so game code can access it easily
	if not ProjectSettings.has_setting("autoload/ItchAPI"):
		add_autoload_singleton("ItchAPI", "res://addons/godot_itch/core/itch_api.gd")
		_autoload_added = true

func _exit_tree() -> void:
	# Remove autoload when plugin is disabled to clean up properly
	if _autoload_added and ProjectSettings.has_setting("autoload/ItchAPI"):
		remove_autoload_singleton("ItchAPI")
		_autoload_added = false
