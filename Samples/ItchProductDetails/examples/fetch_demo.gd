extends Control

@onready var fetch_button: Button = $VBox/FetchButton
@onready var project_line: LineEdit = $VBox/ProjectLine
@onready var status_label: Label = $VBox/StatusLabel
@onready var result: RichTextLabel = $VBox/Result

var auto_demo_timer: Timer

func _ready():
	print("[Demo] Starting simplified fetch demo...")

	# Connect to the ItchAPI autoload or fallback to local instance
	var itch = null
	if has_node("/root/ItchAPI"):
		itch = get_node("/root/ItchAPI")
		print("[Demo] Connected to ItchAPI autoload")
	else:
		print("[Demo] Creating local ItchAPI instance")
		var Itch = preload("res://addons/godot_itch/core/itch_api.gd")
		itch = Itch.new()
		itch.name = "ItchAPI"
		add_child(itch)

	itch.project_fetched.connect(_on_project_fetched)
	itch.fetch_failed.connect(_on_fetch_failed)
	fetch_button.pressed.connect(_on_fetch_pressed)
	print("[Demo] Connected to ItchAPI signals")

	# Auto-demo setup
	project_line.text = "https://jame581.itch.io/stellar-secrets"
	_clear_metadata_display()
	print("[Demo] Auto-demo URL: ", project_line.text)


func _on_fetch_pressed():
	var input = project_line.text.strip_edges()
	print("[Demo] Fetch button pressed, input: '", input, "'")
	if input == "":
		status_label.text = "❌ Please enter a project URL or ID"
		status_label.modulate = Color.RED
		print("[Demo] ERROR: Empty input")
		return

	# UI updates
	status_label.text = "🔄 Fetching project data..."
	status_label.modulate = Color(1,1,1)
	_clear_metadata_display()
	result.scroll_to_line(0)
	fetch_button.disabled = true

	# Get ItchAPI instance (autoload or local)
	var itch = get_node("/root/ItchAPI") if has_node("/root/ItchAPI") else get_node("ItchAPI")
	if itch:
		print("[Demo] Calling ItchAPI.get_public_project with: ", input)
		itch.fetch_public_project(input)
	else:
		print("[Demo] ERROR: Could not find ItchAPI instance")
		status_label.text = "❌ Could not find ItchAPI instance"
		status_label.modulate = Color.RED
		fetch_button.disabled = false

func _on_project_fetched(metadata):
	print("[Demo] SUCCESS: Project metadata received: ", metadata)

	# Update status
	status_label.text = "✅ Project data loaded"
	status_label.modulate = Color(0.2, 0.75, 0.2)
	fetch_button.disabled = false

	# Show formatted metadata in the result area
	# Prefer pretty JSON-like display for readability
	var pretty = ""
	for k in metadata.keys():
		pretty += str(k) + ": " + str(metadata[k]) + "\n\n"

	result.text = pretty if pretty != "" else "(no metadata)"

func _on_fetch_failed(err):
	print("[Demo] ERROR: Fetch failed with error: ", err)
	status_label.text = "❌ Failed to fetch project data"
	status_label.modulate = Color.RED
	fetch_button.disabled = false
	_clear_metadata_display()
	result.text = "Error: " + str(err)

func _clear_metadata_display():
	result.text = ""