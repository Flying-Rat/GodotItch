extends Control

@onready var output: RichTextLabel = $Panel/VBox/Margin/Scroll/Output
@onready var client_id_edit: LineEdit = $Panel/VBox/Inputs/ClientId
@onready var redirect_uri_edit: LineEdit = $Panel/VBox/Inputs/RedirectUri
@onready var state_edit: LineEdit = $Panel/VBox/Inputs/State
@onready var token_edit: LineEdit = $Panel/VBox/Inputs/Token

var auth := ItchAuth.new()

func _ready():
	output.clear()
	_log("=== ItchAuth UI Test ===")
	if auth == null:
		_error("ItchAuth class not found or failed to instantiate")
		return
	_on_init_pressed()
	# Prefill inputs from ProjectSettings
	client_id_edit.text = ProjectSettings.get_setting("godot_itch/oauth_client_id", "")
	redirect_uri_edit.text = ProjectSettings.get_setting("godot_itch/oauth_redirect_uri", "")
	token_edit.text = auth.get_oauth_token()


func _on_save_token_pressed() -> void:
	var token := token_edit.text.strip_edges()
	if token.is_empty():
		_error("OAuth token is required")
		return
	auth.set_oauth_token(token)
	_log("Saved OAuth token: %s" % _mask(token))


func _on_init_pressed() -> void:
	var ok := auth.initialize()
	_log("Init: %s | is_initialized: %s" % [str(ok), str(auth.is_initialized())])


func _on_launch_info_pressed() -> void:
	_log("Launched via itch: %s" % str(auth.is_launched_via_itch()))
	_log("Has API key present: %s" % str(auth.has_api_key_present()))
	if auth.has_api_key_present():
		_log("Launch API key (masked): %s" % _mask(auth.get_launch_api_key()))


func _on_save_oauth_pressed() -> void:
	var cid := client_id_edit.text.strip_edges()
	var ruri := redirect_uri_edit.text.strip_edges()
	if cid.is_empty() or ruri.is_empty():
		_error("client_id and redirect_uri are required")
		return
	auth.set_oauth_client_id(cid)
	auth.set_oauth_redirect_uri(ruri)
	auth.set_oauth_scope("profile:me")
	_log("Saved OAuth settings. client_id=%s redirect_uri=%s" % [cid, ruri])


func _on_show_oauth_pressed() -> void:
	_log("OAuth -> client_id=%s" % auth.get_oauth_client_id())
	_log("OAuth -> redirect_uri=%s" % auth.get_oauth_redirect_uri())
	_log("OAuth -> scope=%s" % auth.get_oauth_scope())


func _on_build_url_pressed() -> void:
	var cid := client_id_edit.text.strip_edges()
	var ruri := redirect_uri_edit.text.strip_edges()
	var state := state_edit.text.strip_edges()
	var url := auth.build_oauth_authorize_url(cid, ruri, state)
	if ruri.is_empty():
		_log("Authorize URL (no redirect_uri): %s" % url)
	else:
		_log("Authorize URL: %s" % url)


func _on_open_browser_pressed() -> void:
	var cid := client_id_edit.text.strip_edges()
	var ruri := redirect_uri_edit.text.strip_edges()
	var state := state_edit.text.strip_edges()
	auth.start_oauth_authorization(cid, ruri, state)
	if ruri.is_empty():
		_log("Opened browser for OAuth authorization (no redirect_uri)")
	else:
		_log("Opened browser for OAuth authorization")


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
