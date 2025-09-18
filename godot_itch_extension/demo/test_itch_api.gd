extends Control

@onready var output: RichTextLabel = $Panel/VBoxContainer/Margin/Scroll/Output
@onready var client_id_edit: LineEdit = $Panel/VBoxContainer/OauthBox/Inputs/ClientId
@onready var redirect_uri_edit: LineEdit = $Panel/VBoxContainer/OauthBox/Inputs/RedirectUri
@onready var state_edit: LineEdit = $Panel/VBoxContainer/OauthBox/Inputs/State
@onready var token_edit: LineEdit = $Panel/VBoxContainer/OauthBox/Inputs/Token

func _ready():
	output.clear()
	
	Itch.initialize_with_scene(self)
	
	var auth = Itch.get_auth()
	_log("=== Auth UI Test ===")
	if auth == null:
		_error("Auth class not found or failed to instantiate")
		return
	_on_init_pressed()
	# Prefill inputs from ProjectSettings
	client_id_edit.text = ProjectSettings.get_setting("godot_itch/oauth_client_id", "")
	redirect_uri_edit.text = ProjectSettings.get_setting("godot_itch/oauth_redirect_uri", "")
	token_edit.text = auth.get_oauth_token()

	# Hook Itch facade signals to show results
	if Itch:
		Itch.api_response.connect(_on_api_response)
		Itch.api_error.connect(_on_api_error)
		Itch.get_auth().auth_result.connect(_on_auth_result)


func _on_save_token_pressed() -> void:
	var token := token_edit.text.strip_edges()
	if token.is_empty():
		_error("OAuth token is required")
		return
	Itch.get_auth().set_oauth_token(token)
	_log("Saved OAuth token: %s" % _mask(token))


func _on_init_pressed() -> void:
	_log("Auth: is_initialized: %s" % str(Itch.get_auth().is_initialized()))


func _on_launch_info_pressed() -> void:
	_log("Launched via itch: %s" % str(Itch.get_auth().is_launched_via_itch()))
	_log("Has API key present: %s" % str(Itch.get_auth().has_api_key_present()))
	if Itch.get_auth().has_api_key_present():
		_log("Launch API key (masked): %s" % _mask(Itch.get_auth().get_launch_api_key()))


func _on_save_oauth_pressed() -> void:
	var cid := client_id_edit.text.strip_edges()
	var ruri := redirect_uri_edit.text.strip_edges()
	var token := token_edit.text.strip_edges()

	if cid.is_empty() or ruri.is_empty():
		_error("client_id and redirect_uri are required")
		return

	if token.is_empty():
		_error("OAuth token is required!")

	Itch.get_auth().set_oauth_client_id(cid)
	Itch.get_auth().set_oauth_redirect_uri(ruri)
	Itch.get_auth().set_oauth_scope("profile:me")
	Itch.get_auth().set_oauth_token(token)
	_log("Saved OAuth settings. client_id=%s redirect_uri=%s" % [cid, ruri])


func _on_show_oauth_pressed() -> void:
	_log("OAuth -> client_id=%s" % Itch.get_auth().get_oauth_client_id())
	_log("OAuth -> redirect_uri=%s" % Itch.get_auth().get_oauth_redirect_uri())
	_log("OAuth -> scope=%s" % Itch.get_auth().get_oauth_scope())


func _on_build_url_pressed() -> void:
	var cid : String = client_id_edit.text.strip_edges()
	var ruri : String = redirect_uri_edit.text.strip_edges()
	var state : String = state_edit.text.strip_edges()
	var url : String = Itch.get_auth().build_oauth_authorize_url(cid, ruri, state)
	if ruri.is_empty():
		_log("Authorize URL (no redirect_uri): %s" % url)
	else:
		_log("Authorize URL: %s" % url)


func _on_open_browser_pressed() -> void:
	var cid := client_id_edit.text.strip_edges()
	var ruri := redirect_uri_edit.text.strip_edges()
	var state := state_edit.text.strip_edges()
	Itch.get_auth().start_oauth_authorization(cid, ruri, state)
	if ruri.is_empty():
		_log("Opened browser for OAuth authorization (no redirect_uri)")
	else:
		_log("Opened browser for OAuth authorization")

func _on_btn_credentials_info_pressed() -> void:
	if not Itch:
		_error("Itch not available")
		return
	Itch.get_credentials_info()
	_log("Requested credentials info")

func _on_btn_get_me_pressed() -> void:
	if not Itch:
		_error("Itch not available")
		return
	Itch.get_me()
	_log("Requested get_me")


func _mask(s: String) -> String:
	if s.is_empty():
		return s
	var visible_chars: int = min(4, s.length())
	return s.left(visible_chars) + "*".repeat(max(0, s.length() - visible_chars))


func _log(msg: String) -> void:
	output.append_text(msg + "\n")


func _error(msg: String) -> void:
	output.append_text("[color=red]" + msg + "[/color]\n")


func _on_btn_validate_token_pressed() -> void:
	var token := token_edit.text.strip_edges()
	if token.is_empty():
		_error("OAuth token is required for validation")
		return
	# Simulate validation (real validation would call Itch API)
	if token.length() > 20:
		_log("Token looks valid (length: %d)" % token.length())
	else:
		_error("Token is too short or invalid")

func _on_api_response(endpoint: String, data: Dictionary) -> void:
	_log("[b]API Response:[/b] %s\n%s" % [endpoint, JSON.stringify(data, "  ")])
	if endpoint == "verify_download_key":
		# Handled also by verify_purchase_result
		pass
	elif endpoint == "get_me":
		var user = data.get("user", {})
		if typeof(user) == TYPE_DICTIONARY:
			var username = user.get("username", "?")
			var display_name = user.get("display_name", "?")
			var uid = str(user.get("id", "?"))
			var url = user.get("url", "")
			_log("Me: %s (%s), id=%s%s" % [username, display_name, uid, url.is_empty() if "" else ", url=" + url])
		else:
			_error("Malformed get_me response: missing user dictionary")
	elif endpoint == "credentials_info":
		var scopes = data.get("scopes", [])
		var expires_at = str(data.get("expires_at", ""))
		_log("Credentials scopes: %s" % JSON.stringify(scopes))
		if not expires_at.is_empty():
			_log("JWT expires_at: %s" % expires_at)
		else:
			_log("No expires_at present (likely API key)")


func _on_api_error(endpoint: String, error_message: String, response_code: int) -> void:
	_error("API Error %s (%d): %s" % [endpoint, response_code, error_message])


func _on_btn_clear_output_pressed() -> void:
	output.text = ""


func _on_auth_result(success : bool, data: Dictionary) -> void:
	print("Auth result success=%s data=%s" % [success, JSON.stringify(data, "  ")])
	output.append_text("Auth result success=%s data=%s" % [success, JSON.stringify(data, "  ")])
