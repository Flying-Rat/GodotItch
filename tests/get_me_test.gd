extends Control

@onready var output: RichTextLabel = $VBox/Scroll/Output
@onready var client_id_edit: LineEdit = $VBox/OAuth/ClientId
@onready var redirect_uri_edit: LineEdit = $VBox/OAuth/RedirectUri
@onready var token_edit: LineEdit = $VBox/OAuth/Token

func _ready():
	Itch.initialize_with_scene(self)
	Itch.api_response.connect(_on_api_response)
	Itch.api_error.connect(_on_api_error)
	client_id_edit.text = ProjectSettings.get_setting("godot_itch/oauth_client_id", "")
	redirect_uri_edit.text = ProjectSettings.get_setting("godot_itch/oauth_redirect_uri", "")
	token_edit.text = Itch.get_auth().get_oauth_token()

func _on_open_oauth_pressed():
	var cid = client_id_edit.text.strip_edges()
	var ruri = redirect_uri_edit.text.strip_edges()
	Itch.get_auth().start_oauth_authorization(cid, ruri, "demo-test")
	output.append_text("Opened OAuth authorization in browser.\n")

func _on_save_token_pressed():
	var token = token_edit.text.strip_edges()
	if token.is_empty():
		output.append_text("[color=red]Token is empty[/color]\n")
		return
	Itch.get_auth().set_oauth_token(token)
	output.append_text("Saved OAuth token.\n")

func _on_run_pressed():
	output.append_text("Requesting get_me...\n")
	Itch.get_me()

func _on_api_response(endpoint: String, data: Dictionary) -> void:
	output.append_text("[b]API Response:[/b] %s\n%s\n" % [endpoint, JSON.stringify(data, "  ")])
	if endpoint == "get_me":
		var user = data.get("user", {})
		if typeof(user) == TYPE_DICTIONARY:
			output.append_text("User: %s (%s), id=%s\n" % [user.get("username","?"), user.get("display_name","?"), str(user.get("id","?"))])

func _on_api_error(endpoint: String, error_message: String, response_code: int) -> void:
	output.append_text("[color=red][b]API Error:[/b] %s (%d): %s[/color]\n" % [endpoint, response_code, error_message])
