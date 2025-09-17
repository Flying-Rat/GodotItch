extends Control

@onready var content_area = $VBox/ContentArea
@onready var top_buttons = $VBox/TopButtons
@onready var placeholder_label = $VBox/ContentArea/PlaceholderLabel

@export var scenes: Array = [
	{ "scene": preload("res://addons/godot_itch/example/example_verification.tscn"), "name": "Verification" },
	{ "scene": preload("res://addons/godot_itch/example/example_oauth.tscn"), "name": "OAuth" }
]

var current_instance = null
var buttons = []
var active_index = 0

func _ready():
	# Clear existing buttons
	for child in top_buttons.get_children():
		child.queue_free()
	
	# Dynamically create buttons
	for i in range(scenes.size()):
		var button = Button.new()
		button.text = scenes[i]["name"]
		button.layout_mode = 2
		button.custom_minimum_size = Vector2(150, 50)  # Make buttons bigger
		top_buttons.add_child(button)
		buttons.append(button)
		button.pressed.connect(_on_button_pressed.bind(i))
	
	# Load first scene by default
	switch_to_scene(0)
	update_button_styles()

func switch_to_scene(index: int):
	clear_content()
	current_instance = scenes[index]["scene"].instantiate()
	content_area.add_child(current_instance)
	placeholder_label.visible = false

func clear_content():
	if current_instance:
		current_instance.queue_free()
		current_instance = null
	placeholder_label.visible = true

func _on_button_pressed(index: int):
	active_index = index
	switch_to_scene(index)
	update_button_styles()

func update_button_styles():
	for i in range(buttons.size()):
		var stylebox = StyleBoxFlat.new()
		if i == active_index:
			# Active button: white border
			stylebox.border_color = Color.WHITE
			stylebox.border_width_left = 2
			stylebox.border_width_top = 2
			stylebox.border_width_right = 2
			stylebox.border_width_bottom = 2
			stylebox.bg_color = Color(0.2, 0.2, 0.2, 1)  # Dark background
			buttons[i].modulate = Color(1, 1, 1, 1)  # Normal brightness
		else:
			# Inactive button: no border
			stylebox.border_width_left = 0
			stylebox.border_width_top = 0
			stylebox.border_width_right = 0
			stylebox.border_width_bottom = 0
			stylebox.bg_color = Color(0.1, 0.1, 0.1, 1)  # Darker background
			buttons[i].modulate = Color(0.7, 0.7, 0.7, 1)  # Dimmed
		
		buttons[i].add_theme_stylebox_override("normal", stylebox)