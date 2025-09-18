extends Control

@onready var output: RichTextLabel = $VBox/Output
@onready var btn_init: Button = $VBox/Actions/BtnInit
@onready var btn_get_me: Button = $VBox/Actions/BtnGetMe

var initialized := false
var auth = null

func _ready() -> void:
	initialization()

	auth = Itch.get_auth()
	output.clear()

	_log("=== Get Me Example ===")
	if auth == null:
		_error("Auth class not found")
		return

	# Provide a quick hint about token availability
	if auth.has_method("get_oauth_token"):
		var tok := str(auth.get_oauth_token())
		if tok.length() > 0:
			_log("OAuth token present (length: %d)" % tok.length())
		else:
			_log("No OAuth token set. You can use the OAuth example to obtain one or launch via the itch app.")

func _on_btn_init_pressed() -> void:
	initialization()

func _on_btn_get_me_pressed() -> void:
	if not initialized:
		_error("Please click Init first to attach HTTP and connect signals.")
		return

	_log("Calling Itch.get_me() ...")
	Itch.get_me()

func _on_api_response(endpoint: String, data: Dictionary) -> void:
	match endpoint:
		"get_me":
			var user: Dictionary = data.get("user", {})
			var id := str(user.get("id", ""))
			var username := str(user.get("username", ""))
			var display_name := str(user.get("display_name", ""))
			_log("get_me: id=%s username=%s display=%s" % [id, username, display_name])
		_:
			_log("Response [" + endpoint + "]: " + JSON.stringify(data))

func _on_api_error(endpoint: String, error_message: String, response_code: int) -> void:
	_error("[%s] (%d) %s" % [endpoint, response_code, error_message])

func _log(msg: String) -> void:
	output.append_text(msg + "\n")

func _error(msg: String) -> void:
	output.append_text("[color=red]" + msg + "[/color]\n")

func initialization() -> void:
	if initialized:
		_log("Already initialized Itch HTTP and connected signals")
		return
	Itch.initialize_with_scene(self)
	if not Itch.api_response.is_connected(_on_api_response):
		Itch.api_response.connect(_on_api_response)
	if not Itch.api_error.is_connected(_on_api_error):
		Itch.api_error.connect(_on_api_error)

	initialized = true
	_log("Initialized Itch HTTP and connected signals")
