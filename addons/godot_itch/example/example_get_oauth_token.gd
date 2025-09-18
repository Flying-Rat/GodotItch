extends Control

@onready var client_id_edit: LineEdit = $VBox/Inputs/ClientId
@onready var redirect_uri_edit: LineEdit = $VBox/Inputs/RedirectUri
@onready var token_edit: LineEdit = $VBox/Inputs/Token
@onready var output: RichTextLabel = $VBox/Output

var auth = null;

func _ready():
	auth = Itch.get_auth();
	output.clear()
	_log("=== OAuth Example ===")
	if auth == null:
		_error("Auth class not found")
		return

	# Initialize HTTP + connect signals so we can call example API methods
	Itch.initialize_with_scene(self)
	if not Itch.api_response.is_connected(_on_api_response):
		Itch.api_response.connect(_on_api_response)
	if not Itch.api_error.is_connected(_on_api_error):
		Itch.api_error.connect(_on_api_error)

	# Prefill from ProjectSettings
	client_id_edit.text = ProjectSettings.get_setting("godot_itch/oauth_client_id", "")
	redirect_uri_edit.text = ProjectSettings.get_setting("godot_itch/oauth_redirect_uri", "")
	token_edit.text = auth.get_oauth_token()

func _on_save_oauth_pressed():
	var cid := client_id_edit.text.strip_edges()
	var ruri := redirect_uri_edit.text.strip_edges()
	if cid.is_empty() or ruri.is_empty():
		_error("Client ID and Redirect URI are required")
		return
	auth.set_oauth_client_id(cid)
	auth.set_oauth_redirect_uri(ruri)
	auth.set_oauth_scope("profile:me")
	_log("Saved OAuth settings")

func _on_open_browser_pressed():
	var cid := client_id_edit.text.strip_edges()
	var ruri := redirect_uri_edit.text.strip_edges()
	auth.start_oauth_authorization(cid, ruri, "")
	_log("Opened browser for OAuth authorization")

func _on_validate_token_pressed():
	var token := token_edit.text.strip_edges()
	if token.is_empty():
		_error("Token is required for validation")
		return
	if token.length() > 20:
		_log("Token appears valid (length: %d)" % token.length())
	else:
		_error("Token is too short")

func _on_save_token_pressed():
	var token := token_edit.text.strip_edges()
	if token.is_empty():
		_error("Token is required")
		return
	auth.set_oauth_token(token)
	_log("Saved OAuth token")

func _on_get_credentials_info_pressed():
	_log("Calling Itch.get_credentials_info() ...")
	Itch.get_credentials_info()

func _on_api_response(endpoint: String, data: Dictionary) -> void:
	match endpoint:
		"credentials_info":
			var scopes = data.get("scopes", [])
			var expires_at = str(data.get("expires_at", ""))
			_log("credentials_info: scopes=%s" % [JSON.stringify(scopes)])
			if not expires_at.is_empty():
				_log("JWT expires_at=" + expires_at)

			_log("Response [" + endpoint + "]: " + JSON.stringify(data))

func _on_api_error(endpoint: String, error_message: String, response_code: int) -> void:
	_error("[%s] (%d) %s" % [endpoint, response_code, error_message])

func _log(msg: String):
	output.append_text(msg + "\n")

func _error(msg: String):
	output.append_text("[color=red]" + msg + "[/color]\n")
