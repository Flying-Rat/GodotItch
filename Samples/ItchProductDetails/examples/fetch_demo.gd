extends Control

@onready var fetch_button: Button = $VBox/InputSection/FetchButton
@onready var project_line: LineEdit = $VBox/InputSection/ProjectLine
@onready var status_label: Label = $VBox/StatusLabel
@onready var project_image: TextureRect = $VBox/ContentArea/LeftPanel/ProjectImage
@onready var image_status: Label = $VBox/ContentArea/LeftPanel/ImageStatus
@onready var title_label: RichTextLabel = $VBox/ContentArea/RightPanel/TitleLabel
@onready var site_label: Label = $VBox/ContentArea/RightPanel/MetaInfo/SiteLabel
@onready var description_label: RichTextLabel = $VBox/ContentArea/RightPanel/DescriptionLabel
@onready var result: RichTextLabel = $VBox/Result

var auto_demo_timer: Timer
var image_request: HTTPRequest

func _ready():
	print("[Demo] Starting fetch demo...")
	
	# Setup image downloader
	image_request = HTTPRequest.new()
	add_child(image_request)
	image_request.request_completed.connect(_on_image_downloaded)
	
	# Connect to the ItchAPI autoload or fallback to local instance
	var itch = null
	if has_node("/root/ItchAPI"):
		itch = get_node("/root/ItchAPI")
		print("[Demo] Connected to ItchAPI autoload")
	else:
		print("[Demo] Creating local ItchAPI instance")
		var Itch = preload("res://addons/godot_itch/itch.gd")
		itch = Itch.new()
		itch.name = "ItchAPI"
		add_child(itch)

	
	itch.project_fetched.connect(_on_project_fetched)
	itch.fetch_failed.connect(_on_fetch_failed)
	fetch_button.pressed.connect(_on_fetch_pressed)
	print("[Demo] Connected to ItchAPI signals")
	
	# Auto-demo setup
	project_line.text = "https://jame581.itch.io/stellar-secrets"
	status_label.text = "Auto-demo starting in 5 seconds..."
	_clear_metadata_display()
	print("[Demo] Auto-demo URL: ", project_line.text)
	
	# Create and start timer for auto-demo
	auto_demo_timer = Timer.new()
	auto_demo_timer.wait_time = 5.0
	auto_demo_timer.one_shot = true
	auto_demo_timer.timeout.connect(_on_auto_demo_timeout)
	add_child(auto_demo_timer)
	auto_demo_timer.start()
	print("[Demo] Auto-demo timer started (5 seconds)")

func _on_fetch_pressed():
	var input = project_line.text.strip_edges()
	print("[Demo] Fetch button pressed, input: '", input, "'")
	if input == "":
		status_label.text = "❌ Please enter a project URL or ID"
		status_label.modulate = Color.RED
		print("[Demo] ERROR: Empty input")
		return
	
	status_label.text = "🔄 Fetching project data..."
	status_label.modulate = Color.WHITE
	_clear_metadata_display()
	result.scroll_to_line(0)
	
	# Get ItchAPI instance (autoload or local)
	var itch = get_node("/root/ItchAPI") if has_node("/root/ItchAPI") else get_node("ItchAPI")
	if itch:
		print("[Demo] Calling ItchAPI.get_public_project with: ", input)
		itch.fetch_public_project(input)
	else:
		print("[Demo] ERROR: Could not find ItchAPI instance")
		status_label.text = "❌ Could not find ItchAPI instance"
		status_label.modulate = Color.RED

func _on_project_fetched(metadata):
	print("[Demo] SUCCESS: Project metadata received: ", metadata)
	
	# Update status
	status_label.text = "✅ Project data loaded successfully"
	status_label.modulate = Color.GREEN
	
	# Update individual labels with clean, modern styling
	if metadata.has("title"):
		title_label.text = "[font_size=20][b]" + metadata["title"] + "[/b][/font_size]"
	else:
		title_label.text = "[color=#888888][i]No title available[/i][/color]"
	
	if metadata.has("site_name"):
		site_label.text = "Source: " + metadata["site_name"]
	else:
		site_label.text = ""
	
	if metadata.has("description"):
		description_label.text = metadata["description"]
	else:
		description_label.text = "[color=#888888][i]No description available[/i][/color]"
	
	# Download and display image
	if metadata.has("image"):
		var image_url = metadata["image"]
		print("[Demo] Downloading image: ", image_url)
		image_status.text = "Downloading image..."
		image_request.request(image_url)
	else:
		project_image.texture = null
		image_status.text = "No image available"
	
	# Show raw metadata in result area with subtle styling
	result.text = "Raw metadata:\n" + str(metadata)

func _on_fetch_failed(err):
	print("[Demo] ERROR: Fetch failed with error: ", err)
	status_label.text = "❌ Failed to fetch project data"
	status_label.modulate = Color.RED
	_clear_metadata_display()
	result.text = "Error: " + str(err)

func _on_image_downloaded(result: int, response_code: int, headers: PackedStringArray, body: PackedByteArray):
	print("[Demo] Image download completed - Result: ", result, ", Response code: ", response_code)
	
	if response_code == 200 and body.size() > 0:
		var image = Image.new()
		var error = image.load_png_from_buffer(body)
		if error != OK:
			error = image.load_jpg_from_buffer(body)
		if error != OK:
			error = image.load_webp_from_buffer(body)
		
		if error == OK:
			var texture = ImageTexture.new()
			texture.set_image(image)
			project_image.texture = texture
			image_status.text = "Image loaded (" + str(image.get_width()) + "x" + str(image.get_height()) + ")"
			print("[Demo] Image loaded successfully: ", image.get_width(), "x", image.get_height())
		else:
			image_status.text = "Failed to decode image"
			print("[Demo] Failed to decode image, error: ", error)
	else:
		image_status.text = "Failed to download image"
		print("[Demo] Failed to download image, response code: ", response_code)

func _clear_metadata_display():
	title_label.text = ""
	site_label.text = ""
	description_label.text = ""
	project_image.texture = null
	image_status.text = "No image"
	result.text = ""

func _on_auto_demo_timeout():
	# Trigger the fetch automatically after 5 seconds
	print("[Demo] Auto-demo timeout triggered, starting fetch...")
	status_label.text = "🚀 Auto-fetching project data..."
	status_label.modulate = Color.WHITE
	_on_fetch_pressed()
