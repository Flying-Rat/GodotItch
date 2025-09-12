#include "godotitch.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static Itch *s_singleton = nullptr;

void Itch::_bind_methods()
{
	// API methods
	ClassDB::bind_method(D_METHOD("get_me"), &Itch::get_me);
	ClassDB::bind_method(D_METHOD("get_my_games"), &Itch::get_my_games);
	ClassDB::bind_method(D_METHOD("test_request_http"), &Itch::test_request_http);
	ClassDB::bind_method(D_METHOD("get_game_purchases", "game_id"), &Itch::get_game_purchases, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_game_uploads", "game_id"), &Itch::get_game_uploads, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_download_key", "download_key", "game_id"), &Itch::get_download_key, DEFVAL(""), DEFVAL(""));

	// Itch.io API wrappers
	ClassDB::bind_method(D_METHOD("verify_purchase", "download_key"), &Itch::verify_purchase);

	// Utility methods
	ClassDB::bind_method(D_METHOD("set_api_key", "api_key"), &Itch::set_api_key);
	ClassDB::bind_method(D_METHOD("set_game_id", "game_id"), &Itch::set_game_id);
	ClassDB::bind_method(D_METHOD("get_api_key"), &Itch::get_api_key);
	ClassDB::bind_method(D_METHOD("get_game_id"), &Itch::get_game_id);
	ClassDB::bind_method(D_METHOD("get_godotitch_version"), &Itch::get_godotitch_version);

	// Scene management
	ClassDB::bind_method(D_METHOD("initialize_with_scene", "scene_node"), &Itch::initialize_with_scene);

	// HTTP callback
	ClassDB::bind_method(D_METHOD("_on_request_completed", "result", "response_code", "headers", "body"), &Itch::_on_request_completed);
	// Internal helper to perform HTTP request deferred
	ClassDB::bind_method(D_METHOD("_perform_request", "url", "headers"), &Itch::_perform_request);
	ClassDB::bind_method(D_METHOD("post_request_check"), &Itch::post_request_check);

	// New local hook for api_response
	ClassDB::bind_method(D_METHOD("_on_api_response", "endpoint", "data"), &Itch::_on_api_response);

	// OAuth helpers
	ClassDB::bind_method(D_METHOD("set_oauth_client_id", "client_id"), &Itch::set_oauth_client_id);
	ClassDB::bind_method(D_METHOD("set_oauth_redirect_uri", "redirect_uri"), &Itch::set_oauth_redirect_uri);
	ClassDB::bind_method(D_METHOD("set_oauth_scope", "scope"), &Itch::set_oauth_scope);
	ClassDB::bind_method(D_METHOD("get_oauth_client_id"), &Itch::get_oauth_client_id);
	ClassDB::bind_method(D_METHOD("get_oauth_redirect_uri"), &Itch::get_oauth_redirect_uri);
	ClassDB::bind_method(D_METHOD("get_oauth_scope"), &Itch::get_oauth_scope);
	ClassDB::bind_method(D_METHOD("build_oauth_authorize_url", "client_id", "redirect_uri", "state"), &Itch::build_oauth_authorize_url, DEFVAL(""), DEFVAL(""), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("start_oauth_authorization", "client_id", "redirect_uri", "state"), &Itch::start_oauth_authorization, DEFVAL(""), DEFVAL(""), DEFVAL(""));

	// Signals
	ADD_SIGNAL(MethodInfo("api_response", PropertyInfo(Variant::STRING, "endpoint"), PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("api_error", PropertyInfo(Variant::STRING, "endpoint"), PropertyInfo(Variant::STRING, "error_message"), PropertyInfo(Variant::INT, "response_code")));
	ADD_SIGNAL(MethodInfo("verify_purchase_result", PropertyInfo(Variant::BOOL, "is_verified"), PropertyInfo(Variant::DICTIONARY, "data")));
}

Itch::Itch()
{
	ensure_project_settings();
	// Don't create HTTPRequest here - wait for initialize_with_scene()
	s_singleton = this;
	data_store = ItchDataStore::get_singleton();

	// Connect our own api_response signal to local handler
	connect("api_response", Callable(this, "_on_api_response"));
}

Itch::~Itch()
{
	if (http_request)
	{
		http_request->queue_free();
	}
	if (data_store)
	{
		data_store->shutdown();
	}
	if (s_singleton == this)
		s_singleton = nullptr;
}

Itch *Itch::get_singleton()
{
	return s_singleton;
}

bool Itch::itchInitEx(uint32_t app_id, bool embed_callbacks)
{
	is_initialized = true;
	return true;
}

void Itch::ensure_project_settings()
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps)
		return;
	if (!ps->has_setting(SETTING_API_KEY))
	{
		ps->set_setting(SETTING_API_KEY, "");
	}
	if (!ps->has_setting(SETTING_GAME_ID))
	{
		ps->set_setting(SETTING_GAME_ID, "");
	}
	if (!ps->has_setting(SETTING_OAUTH_CLIENT_ID)) {
		ps->set_setting(SETTING_OAUTH_CLIENT_ID, "");
	}
	if (!ps->has_setting(SETTING_OAUTH_REDIRECT_URI)) {
		ps->set_setting(SETTING_OAUTH_REDIRECT_URI, "");
	}
	if (!ps->has_setting(SETTING_OAUTH_SCOPE)) {
		// The only supported scope is "profile:me"
		ps->set_setting(SETTING_OAUTH_SCOPE, "profile:me");
	}
}

String Itch::get_api_key_from_settings() const
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps)
		return "";
	Variant v = ps->get_setting(SETTING_API_KEY);
	if (v.get_type() == Variant::STRING)
		return v;
	return "";
}

String Itch::get_game_id_from_settings() const
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps)
		return "";
	Variant v = ps->get_setting(SETTING_GAME_ID);
	if (v.get_type() == Variant::STRING)
		return v;
	return "";
}

// OAuth settings setters
void Itch::set_oauth_client_id(const String &client_id)
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps) {
		ps->set_setting(SETTING_OAUTH_CLIENT_ID, client_id);
	}
}
void Itch::set_oauth_redirect_uri(const String &redirect_uri)
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps) {
		ps->set_setting(SETTING_OAUTH_REDIRECT_URI, redirect_uri);
	}
}
void Itch::set_oauth_scope(const String &scope)
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps) {
		ps->set_setting(SETTING_OAUTH_SCOPE, scope);
	}
}

// OAuth settings getters
String Itch::get_oauth_client_id() const
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return "";
	Variant v = ps->get_setting(SETTING_OAUTH_CLIENT_ID);
	return v.get_type() == Variant::STRING ? (String)v : "";
}
String Itch::get_oauth_redirect_uri() const
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return "";
	Variant v = ps->get_setting(SETTING_OAUTH_REDIRECT_URI);
	return v.get_type() == Variant::STRING ? (String)v : "";
}
String Itch::get_oauth_scope() const
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return "profile:me";
	Variant v = ps->get_setting(SETTING_OAUTH_SCOPE);
	String s = v.get_type() == Variant::STRING ? (String)v : "profile:me";
	// The only allowed scope is profile:me. Enforce if misconfigured.
	if (s != "profile:me") {
		s = "profile:me";
	}
	return s;
}

// Build OAuth authorization URL
String Itch::build_oauth_authorize_url(const String &client_id, const String &redirect_uri, const String &state) const
{
	String cid = client_id.is_empty() ? get_oauth_client_id() : client_id;
	String ruri = redirect_uri.is_empty() ? get_oauth_redirect_uri() : redirect_uri;
	String scope = get_oauth_scope(); // enforced to "profile:me"

	if (cid.is_empty() || ruri.is_empty()) {
		UtilityFunctions::push_error("OAuth client_id and redirect_uri must be set (either via parameters or project settings).");
		return "";
	}

	// Encode parameters
	String cid_enc = cid.uri_encode();
	String ruri_enc = ruri.uri_encode();
	String scope_enc = scope.uri_encode();
	String url = "https://itch.io/user/oauth?client_id=" + cid_enc + "&scope=" + scope_enc + "&redirect_uri=" + ruri_enc;
	if (!state.is_empty()) {
		url += "&state=" + state.uri_encode();
	}
	return url;
}

// Open OAuth authorization URL in system browser
void Itch::start_oauth_authorization(const String &client_id, const String &redirect_uri, const String &state)
{
	String url = build_oauth_authorize_url(client_id, redirect_uri, state);
	if (url.is_empty()) {
		return;
	}
	OS *os = OS::get_singleton();
	if (os) {
		bool ok = os->shell_open(url) == Error::OK;
		if (!ok) {
			UtilityFunctions::push_error("Failed to open OAuth authorization URL in browser.");
		}
	}
}

void Itch::_setup_http_request()
{
	if (!http_request)
	{
		http_request = memnew(HTTPRequest);
		// HTTPRequest will be added to scene tree via initialize_with_scene()
		if (http_request)
		{
			http_request->connect("request_completed", Callable(this, "_on_request_completed"));
			UtilityFunctions::print("Itch: HTTPRequest object created");
			// Configure safer defaults
			http_request->set_use_threads(false);
			http_request->set_timeout(10.0);
			http_request->set_accept_gzip(true);
			http_request->set_name("ItchHTTPRequest");
			UtilityFunctions::print(String("Itch: HTTPRequest use_threads=") + (http_request->is_using_threads() ? "true" : "false"));
		}
	}
}

String Itch::_build_api_url(const String &endpoint) const
{
	String api_key = get_api_key_from_settings();
	if (api_key.is_empty())
	{
		UtilityFunctions::push_error("Itch.io API key not set in project settings");
		return "";
	}
	return "https://itch.io/api/1/" + api_key + endpoint;
}

// API Methods
void Itch::get_me()
{
	if (!http_request)
	{
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}

	String url = _build_api_url("/me");
	if (url.is_empty())
		return;

	pending_request_type = "get_me";
	pending_request_data.clear();

	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	// Diagnostics: log whether HTTPRequest is inside scene tree
	if (http_request->is_inside_tree())
	{
		UtilityFunctions::print("Itch: HTTPRequest is inside scene tree, issuing request");
	}
	else
	{
		UtilityFunctions::print("Itch: HTTPRequest NOT inside scene tree, deferring request");
	}

	// Use call_deferred so the request is executed after the node is in the scene tree.
	// Schedule an internal deferred method which will call HTTPRequest::request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::test_request_http()
{
	if (!http_request)
	{
		UtilityFunctions::print("Itch: test_request_http - http_request not initialized");
		return;
	}
	String url = "http://example.com/";
	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	call_deferred("_perform_request", url, headers);
}

void Itch::get_my_games()
{
	if (!http_request)
	{
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}

	String url = _build_api_url("/my-games");
	if (url.is_empty())
		return;

	pending_request_type = "get_my_games";
	pending_request_data.clear();

	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");

	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::get_game_purchases(const String &game_id)
{
	if (!http_request)
	{
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}

	String target_game_id = game_id.is_empty() ? get_game_id_from_settings() : game_id;
	if (target_game_id.is_empty())
	{
		UtilityFunctions::push_error("Game ID not provided and not set in project settings");
		return;
	}

	String url = _build_api_url("/game/" + target_game_id + "/purchases");
	if (url.is_empty())
		return;

	pending_request_type = "get_game_purchases";
	pending_request_data.clear();
	pending_request_data["game_id"] = target_game_id;

	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");

	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::get_game_uploads(const String &game_id)
{
	if (!http_request)
	{
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}

	String target_game_id = game_id.is_empty() ? get_game_id_from_settings() : game_id;
	if (target_game_id.is_empty())
	{
		UtilityFunctions::push_error("Game ID not provided and not set in project settings");
		return;
	}

	String url = _build_api_url("/game/" + target_game_id + "/uploads");
	if (url.is_empty())
		return;

	pending_request_type = "get_game_uploads";
	pending_request_data.clear();
	pending_request_data["game_id"] = target_game_id;

	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");

	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::get_download_key(const String &download_key, const String &game_id)
{
	if (!http_request)
	{
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}

	if (download_key.is_empty())
	{
		UtilityFunctions::push_error("Download key must be provided");
		return;
	}

	String target_game_id = game_id.is_empty() ? get_game_id_from_settings() : game_id;
	if (target_game_id.is_empty())
	{
		UtilityFunctions::push_error("Game ID must be provided or set in project settings");
		return;
	}

	String url = _build_api_url("/game/" + target_game_id + "/download_keys?download_key=" + download_key);
	if (url.is_empty())
		return;

	pending_request_type = "get_download_key";
	pending_request_data.clear();
	pending_request_data["download_key"] = download_key;
	pending_request_data["game_id"] = game_id;

	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");

	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::verify_purchase(const String &download_key)
{
	if (download_key.is_empty())
	{
		UtilityFunctions::push_error("Download key must be provided for verify_purchase");
		return;
	}

	// Check if already verified
	if (data_store && data_store->is_verified(download_key))
	{
		Dictionary data = data_store->get_verification_data(download_key);
		emit_signal("verify_purchase_result", true, data);
		return;
	}

	if (!http_request)
	{
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}

	String target_game_id = get_game_id_from_settings();
	if (target_game_id.is_empty())
	{
		UtilityFunctions::push_error("Game ID must be provided or set in project settings");
		return;
	}

	String url = _build_api_url("/game/" + target_game_id + "/download_keys?download_key=" + download_key);
	if (url.is_empty())
		return;

	pending_request_type = "verify_download_key";
	pending_request_data.clear();
	pending_request_data["download_key"] = download_key;
	pending_request_data["game_id"] = target_game_id;

	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
}

void Itch::_perform_request(const String &url, const PackedStringArray &headers)
{
	if (!http_request)
	{
		UtilityFunctions::print("Itch: _perform_request called but http_request is null");
		return;
	}
	if (!http_request->is_inside_tree())
	{
		UtilityFunctions::print("Itch: _perform_request - HTTPRequest not yet inside tree, deferring again");
		call_deferred("_perform_request", url, headers);
		return;
	}

	UtilityFunctions::print(String("Itch: _perform_request issuing request to: ") + url);
	// Print pointer address for diagnostics
	UtilityFunctions::print(String("Itch: http_request ptr: ") + String::num_int64((int64_t)http_request));
	// Invoke via Variant call to route through Godot's method binding layer
	Variant ret = http_request->call("request", url, headers);
	// Log return value if any
	if (ret.get_type() != Variant::NIL)
	{
		UtilityFunctions::print(String("Itch: HTTPRequest::request returned variant type: ") + String::num_int64((int64_t)ret.get_type()));
		// also print numeric value if convertible
		int64_t num = 0;
		if (ret.get_type() == Variant::INT)
		{
			num = (int64_t)ret;
			UtilityFunctions::print(String("Itch: HTTPRequest::request returned numeric: ") + String::num_int64(num));
		}
	}
	else
	{
		UtilityFunctions::print("Itch: HTTPRequest::request returned nil");
	}

	// Schedule a deferred post-request check to see if node is still valid
	call_deferred("post_request_check");
}

void Itch::post_request_check()
{
	if (!http_request)
	{
		UtilityFunctions::print("Itch: post_request_check - http_request is null");
		return;
	}
	UtilityFunctions::print(String("Itch: post_request_check - http_request ptr: ") + String::num_int64((int64_t)http_request));
	UtilityFunctions::print(String("Itch: post_request_check - is_inside_tree: ") + (http_request->is_inside_tree() ? "true" : "false"));
}

// Utility Methods
void Itch::set_api_key(const String &api_key)
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps)
	{
		ps->set_setting(SETTING_API_KEY, api_key);
	}
}

void Itch::set_game_id(const String &game_id)
{
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps)
	{
		ps->set_setting(SETTING_GAME_ID, game_id);
	}
}

String Itch::get_api_key() const
{
	return get_api_key_from_settings();
}

String Itch::get_game_id() const
{
	return get_game_id_from_settings();
}

void Itch::initialize_with_scene(Node *scene_node)
{
	if (!http_request && scene_node)
	{
		_setup_http_request();
		if (http_request)
		{
			UtilityFunctions::print("Itch: Adding HTTPRequest to scene_node");
			scene_node->add_child(http_request);
			UtilityFunctions::print(String("Itch: HTTPRequest is_inside_tree after add_child: ") + String(http_request->is_inside_tree() ? "true" : "false"));
		}
	}
}

void Itch::_on_request_completed(int result, int response_code, const PackedStringArray &headers, const PackedByteArray &body)
{
	String body_string = body.get_string_from_utf8();

	if (response_code != 200)
	{
		emit_signal("api_error", pending_request_type, "HTTP Error: " + String::num_int64(response_code), response_code);
		return;
	}

	Variant parsed = JSON::parse_string(body_string);
	if (parsed.get_type() == Variant::NIL)
	{
		emit_signal("api_error", pending_request_type, "Failed to parse JSON response", response_code);
		return;
	}
	Dictionary response_data;
	if (parsed.get_type() == Variant::DICTIONARY)
	{
		response_data = (Dictionary)parsed;
	}
	else
	{
		// Wrap non-dictionary JSON into a result container for consistency
		response_data["result"] = parsed;
	}

	// Add request metadata to response
	response_data["_request_type"] = pending_request_type;
	if (!pending_request_data.is_empty())
	{
		response_data["_request_data"] = pending_request_data;
	}

	emit_signal("api_response", pending_request_type, response_data);
}

void Itch::_on_api_response(const String &endpoint, const Dictionary &data)
{
	// Derive verification result for download key requests
	bool is_verify_type = endpoint == String("verify_download_key");
	if (!is_verify_type) {
		return;
	}

	bool verified = false;
	// For itch.io download key endpoint, success usually includes a "download_key" object
	if (data.has("download_key")) {
		Variant dk = data["download_key"];
		// Consider presence of object as verification success
		verified = dk.get_type() == Variant::DICTIONARY || dk.get_type() == Variant::OBJECT;
	}
	// Fallback: If HTTP handled non-dict, check a generic "result"
	if (!verified && data.has("result")) {
		verified = true; // any result treated as success here
	}

	// Save verification result if verified
	if (verified && data_store) {
		String download_key = pending_request_data["download_key"];
		data_store->set_verified(download_key, verified, data);
	}

	emit_signal("verify_purchase_result", verified, data);
}