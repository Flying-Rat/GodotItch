extends Node2D

@onready var itch_api_tests : Control = $CanvasLayer/TestItchApi
@onready var itch_auth_tests : Control = $CanvasLayer/TestItchAuth

func _ready() -> void:
	itch_api_tests.visible = true
	itch_auth_tests.visible = false

func _on_itch_api_tests_button_pressed() -> void:
	itch_api_tests.visible = true
	itch_auth_tests.visible = false


func _on_itch_auth_tests_button_pressed() -> void:
	itch_api_tests.visible = false
	itch_auth_tests.visible = true
