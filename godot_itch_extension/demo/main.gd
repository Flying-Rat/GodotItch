extends Node2D

@onready var itch_api_tests : Control = $CanvasLayer/TestItchApi
@onready var auth_tests : Control = $CanvasLayer/TestAuth

func _ready() -> void:
	itch_api_tests.visible = true
	auth_tests.visible = false

func _on_itch_api_tests_button_pressed() -> void:
	itch_api_tests.visible = true
	auth_tests.visible = false


func _on_itch_auth_tests_button_pressed() -> void:
	itch_api_tests.visible = false
	auth_tests.visible = true
