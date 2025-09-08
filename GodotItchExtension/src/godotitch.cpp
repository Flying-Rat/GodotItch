#include "godotitch.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/script.hpp>

using namespace godot;

static Itch *s_singleton = nullptr;


void Itch::_bind_methods() {
	// API methods
	ClassDB::bind_method(D_METHOD("get_me"), &Itch::get_me);
	ClassDB::bind_method(D_METHOD("get_my_games"), &Itch::get_my_games);
	ClassDB::bind_method(D_METHOD("test_request_http"), &Itch::test_request_http);
	ClassDB::bind_method(D_METHOD("get_game_purchases", "game_id"), &Itch::get_game_purchases, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("get_game_uploads", "game_id"), &Itch::get_game_uploads, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("verify_user", "username"), &Itch::verify_user);
	
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
	
	// Signals
	ADD_SIGNAL(MethodInfo("api_response", PropertyInfo(Variant::STRING, "endpoint"), PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("api_error", PropertyInfo(Variant::STRING, "endpoint"), PropertyInfo(Variant::STRING, "error_message"), PropertyInfo(Variant::INT, "response_code")));
}


Itch::Itch() {
	ensure_project_settings();
	// Don't create HTTPRequest here - wait for initialize_with_scene()
	s_singleton = this;
}


Itch::~Itch() {
	if (http_request) {
		http_request->queue_free();
	}
	if (s_singleton == this) s_singleton = nullptr;
}

Itch *Itch::get_singleton() {
	return s_singleton;
}

bool Itch::itchInitEx(uint32_t app_id, bool embed_callbacks) {
	is_initialized = true;
	return true;
}

void Itch::ensure_project_settings() {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return;
	if (!ps->has_setting(SETTING_API_KEY)) {
		ps->set_setting(SETTING_API_KEY, "");
	}
	if (!ps->has_setting(SETTING_GAME_ID)) {
		ps->set_setting(SETTING_GAME_ID, "");
	}
}

String Itch::get_api_key_from_settings() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return "";
	Variant v = ps->get_setting(SETTING_API_KEY);
	if (v.get_type() == Variant::STRING)
		return v;
	return "";
}

String Itch::get_game_id_from_settings() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return "";
	Variant v = ps->get_setting(SETTING_GAME_ID);
	if (v.get_type() == Variant::STRING)
		return v;
	return "";
}

void Itch::_setup_http_request() {
	if (!http_request) {
		http_request = memnew(HTTPRequest);
		// HTTPRequest will be added to scene tree via initialize_with_scene()
		if (http_request) {
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

String Itch::_build_api_url(const String& endpoint) const {
	String api_key = get_api_key_from_settings();
	if (api_key.is_empty()) {
		UtilityFunctions::push_error("Itch.io API key not set in project settings");
		return "";
	}
	return "https://itch.io/api/1/" + api_key + endpoint;
}

// API Methods
void Itch::get_me() {
	if (!http_request) {
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}
	
	String url = _build_api_url("/me");
	if (url.is_empty()) return;
	
	pending_request_type = "get_me";
	pending_request_data.clear();
	
	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	// Diagnostics: log whether HTTPRequest is inside scene tree
	if (http_request->is_inside_tree()) {
		UtilityFunctions::print("Itch: HTTPRequest is inside scene tree, issuing request");
	} else {
		UtilityFunctions::print("Itch: HTTPRequest NOT inside scene tree, deferring request");
	}
	
	// Use call_deferred so the request is executed after the node is in the scene tree.
	// Schedule an internal deferred method which will call HTTPRequest::request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::test_request_http() {
	if (!http_request) {
		UtilityFunctions::print("Itch: test_request_http - http_request not initialized");
		return;
	}
	String url = "http://example.com/";
	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	call_deferred("_perform_request", url, headers);
}

void Itch::get_my_games() {
	if (!http_request) {
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}
	
	String url = _build_api_url("/my-games");
	if (url.is_empty()) return;
	
	pending_request_type = "get_my_games";
	pending_request_data.clear();
	
	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	
	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::get_game_purchases(const String& game_id) {
	if (!http_request) {
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}
	
	String target_game_id = game_id.is_empty() ? get_game_id_from_settings() : game_id;
	if (target_game_id.is_empty()) {
		UtilityFunctions::push_error("Game ID not provided and not set in project settings");
		return;
	}
	
	String url = _build_api_url("/game/" + target_game_id + "/purchases");
	if (url.is_empty()) return;
	
	pending_request_type = "get_game_purchases";
	pending_request_data.clear();
	pending_request_data["game_id"] = target_game_id;
	
	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	
	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::get_game_uploads(const String& game_id) {
	if (!http_request) {
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}
	
	String target_game_id = game_id.is_empty() ? get_game_id_from_settings() : game_id;
	if (target_game_id.is_empty()) {
		UtilityFunctions::push_error("Game ID not provided and not set in project settings");
		return;
	}
	
	String url = _build_api_url("/game/" + target_game_id + "/uploads");
	if (url.is_empty()) return;
	
	pending_request_type = "get_game_uploads";
	pending_request_data.clear();
	pending_request_data["game_id"] = target_game_id;
	
	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	
	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::verify_user(const String& username) {
	if (!http_request) {
		UtilityFunctions::push_error("HTTPRequest not initialized");
		return;
	}
	
	String url = _build_api_url("/user/" + username);
	if (url.is_empty()) return;
	
	pending_request_type = "verify_user";
	pending_request_data.clear();
	pending_request_data["username"] = username;
	
	PackedStringArray headers;
	headers.push_back("User-Agent: GodotItch/1.0");
	
	// Schedule deferred internal helper to perform the request
	call_deferred("_perform_request", url, headers);
	UtilityFunctions::print("Itch: Deferred internal request scheduled");
}

void Itch::_perform_request(const String &url, const PackedStringArray &headers) {
	SceneTree *tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
	if (!tree) {
		UtilityFunctions::print("Itch: Could not get SceneTree");
		return;
	}
	Node *scene_root = tree->get_current_scene();
	if (!scene_root) {
		UtilityFunctions::print("Itch: Could not get current scene root");
		return;
	}
	Node *helper = scene_root->get_node_or_null("HttpRequestHelper");
	if (!helper) {
		helper = memnew(Node);
		helper->set_name("HttpRequestHelper");
		// Load GDScript as Resource and cast to Script
		Ref<Resource> res = ResourceLoader::load("res://demo/HttpRequestHelper.gd");
		Ref<Script> script;
		if (res.is_valid()) {
			Script *script_ptr = Object::cast_to<Script>(res.ptr());
			script = Ref<Script>(script_ptr);
		}
		if (script.is_valid()) {
			helper->set_script(script);
			scene_root->add_child(helper);
		} else {
			UtilityFunctions::print("Itch: Could not load HttpRequestHelper.gd as Script");
			return;
		}
	}
	// Connect signal if not already
	if (!helper->is_connected("request_completed", Callable(this, "_on_gdscript_request_completed"))) {
		helper->connect("request_completed", Callable(this, "_on_gdscript_request_completed"));
	}
	UtilityFunctions::print("Itch: Delegating HTTP request to GDScript helper");
	helper->call("perform_request", url, headers);
}

void Itch::_on_gdscript_request_completed(int result, int response_code, const PackedStringArray &headers, const PackedByteArray &body) {
	UtilityFunctions::print("Itch: Received request_completed from GDScript helper");
	_on_request_completed(result, response_code, headers, body);
}

void Itch::post_request_check() {
	if (!http_request) {
		UtilityFunctions::print("Itch: post_request_check - http_request is null");
		return;
	}
	UtilityFunctions::print(String("Itch: post_request_check - http_request ptr: ") + String::num_int64((int64_t)http_request));
	UtilityFunctions::print(String("Itch: post_request_check - is_inside_tree: ") + (http_request->is_inside_tree() ? "true" : "false"));
}

// Utility Methods
void Itch::set_api_key(const String& api_key) {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps) {
		ps->set_setting(SETTING_API_KEY, api_key);
	}
}

void Itch::set_game_id(const String& game_id) {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps) {
		ps->set_setting(SETTING_GAME_ID, game_id);
	}
}

String Itch::get_api_key() const {
	return get_api_key_from_settings();
}

String Itch::get_game_id() const {
	return get_game_id_from_settings();
}

void Itch::initialize_with_scene(Node* scene_node) {
	if (!http_request && scene_node) {
		_setup_http_request();
		if (http_request) {
			UtilityFunctions::print("Itch: Adding HTTPRequest to scene_node");
			scene_node->add_child(http_request);
			UtilityFunctions::print(String("Itch: HTTPRequest is_inside_tree after add_child: ") + String(http_request->is_inside_tree() ? "true" : "false"));
		}
	}
}

void Itch::_on_request_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body) {
	String body_string = body.get_string_from_utf8();
	
	if (response_code != 200) {
		emit_signal("api_error", pending_request_type, "HTTP Error: " + String::num_int64(response_code), response_code);
		return;
	}
	
	JSON json;
	Error parse_result = json.parse(body_string);
	
	if (parse_result != OK) {
		emit_signal("api_error", pending_request_type, "Failed to parse JSON response", response_code);
		return;
	}
	
	Dictionary response_data = json.get_data();
	
	// Add request metadata to response
	response_data["_request_type"] = pending_request_type;
	if (!pending_request_data.is_empty()) {
		response_data["_request_data"] = pending_request_data;
	}
	
	emit_signal("api_response", pending_request_type, response_data);
}