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
	// Itch.io API wrappers
	ClassDB::bind_method(D_METHOD("verify_download_key", "download_key"), &Itch::verify_download_key);

	// Utility methods
	ClassDB::bind_method(D_METHOD("set_game_id", "game_id"), &Itch::set_game_id);
	ClassDB::bind_method(D_METHOD("get_game_id"), &Itch::get_game_id);

	// Launch detection getters
	ClassDB::bind_method(D_METHOD("is_launched_via_itch"), &Itch::is_launched_via_itch);
	ClassDB::bind_method(D_METHOD("has_api_key_present"), &Itch::has_api_key_present);
	ClassDB::bind_method(D_METHOD("get_godotitch_version"), &Itch::get_godotitch_version);

	// Module access - expose all subsystems
	ClassDB::bind_method(D_METHOD("get_entitlements"), &Itch::get_entitlements);
	ClassDB::bind_method(D_METHOD("get_core"), &Itch::get_core);
	ClassDB::bind_method(D_METHOD("get_auth"), &Itch::get_auth);
	ClassDB::bind_method(D_METHOD("get_user"), &Itch::get_user);
	ClassDB::bind_method(D_METHOD("get_games"), &Itch::get_games);

	// Scene management
	ClassDB::bind_method(D_METHOD("initialize_with_scene", "scene_node"), &Itch::initialize_with_scene);

	// HTTP callback
	ClassDB::bind_method(D_METHOD("_on_request_completed", "result", "response_code", "headers", "body"), &Itch::_on_request_completed);
	// Internal helper to perform HTTP request deferred
	ClassDB::bind_method(D_METHOD("_perform_request", "url", "headers"), &Itch::_perform_request);
	ClassDB::bind_method(D_METHOD("post_request_check"), &Itch::post_request_check);

	// New local hook for api_response
	ClassDB::bind_method(D_METHOD("_on_api_response", "endpoint", "data"), &Itch::_on_api_response);
	// Entitlements signal handlers
	ClassDB::bind_method(D_METHOD("_on_entitlement_verified", "success", "data"), &Itch::_on_entitlement_verified);
	ClassDB::bind_method(D_METHOD("_on_entitlement_error", "error_message"), &Itch::_on_entitlement_error);

	// OAuth hooks (for integration to call after external flow)
	ClassDB::bind_method(D_METHOD("oauth_login_success", "user"), &Itch::oauth_login_success);
	ClassDB::bind_method(D_METHOD("oauth_login_failed", "error"), &Itch::oauth_login_failed);
	ClassDB::bind_method(D_METHOD("oauth_logged_out"), &Itch::oauth_logged_out);

	// OAuth helpers (delegated to Auth)
	ClassDB::bind_method(D_METHOD("set_oauth_client_id", "client_id"), &Itch::set_oauth_client_id);
	ClassDB::bind_method(D_METHOD("set_oauth_redirect_uri", "redirect_uri"), &Itch::set_oauth_redirect_uri);
	ClassDB::bind_method(D_METHOD("set_oauth_scope", "scope"), &Itch::set_oauth_scope);
	ClassDB::bind_method(D_METHOD("get_oauth_client_id"), &Itch::get_oauth_client_id);
	ClassDB::bind_method(D_METHOD("get_oauth_redirect_uri"), &Itch::get_oauth_redirect_uri);
	ClassDB::bind_method(D_METHOD("get_oauth_scope"), &Itch::get_oauth_scope);
	ClassDB::bind_method(D_METHOD("build_oauth_authorize_url", "client_id", "redirect_uri", "state"),
						 &Itch::build_oauth_authorize_url, DEFVAL(""), DEFVAL(""), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("start_oauth_authorization", "client_id", "redirect_uri", "state"),
						 &Itch::start_oauth_authorization, DEFVAL(""), DEFVAL(""), DEFVAL(""));

	ClassDB::bind_method(D_METHOD("is_user_authenticated"), &Itch::is_user_authenticated);
	ClassDB::bind_method(D_METHOD("get_current_user"), &Itch::get_current_user);
	ClassDB::bind_method(D_METHOD("get_current_user_name"), &Itch::get_current_user_name);
	ClassDB::bind_method(D_METHOD("get_credentials_info"), &Itch::get_credentials_info);

	// Signals
	ADD_SIGNAL(MethodInfo("api_response", PropertyInfo(Variant::STRING, "endpoint"),
						  PropertyInfo(Variant::DICTIONARY, "data")));
	ADD_SIGNAL(MethodInfo("api_error", PropertyInfo(Variant::STRING, "endpoint"),
	                      PropertyInfo(Variant::STRING, "error_message"),
	                      PropertyInfo(Variant::INT, "response_code")));
	ADD_SIGNAL(MethodInfo("verify_download_key_result", PropertyInfo(Variant::BOOL, "is_verified"),
	                      PropertyInfo(Variant::DICTIONARY, "data")));
	// Auth signals
	ADD_SIGNAL(MethodInfo("user_logged_in", PropertyInfo(Variant::DICTIONARY, "user")));
	ADD_SIGNAL(MethodInfo("user_logged_out"));
	ADD_SIGNAL(MethodInfo("user_login_failed", PropertyInfo(Variant::STRING, "error")));
}

Itch::Itch()
{
	// Initialize modular architecture
	// Core and Auth are already initialized in register_types.cpp

	// Don't create HTTPRequest here - wait for initialize_with_scene()
	s_singleton = this;
	auth = Auth::get_singleton();

	// Connect our own api_response signal to local handler
	connect("api_response", Callable(this, "_on_api_response"));

	// Connect entitlements signals to facade methods
	Entitlements *entitlements = Entitlements::get_singleton();
	if (entitlements)
	{
		entitlements->connect("entitlement_verified", Callable(this, "_on_entitlement_verified"));
		entitlements->connect("entitlement_error", Callable(this, "_on_entitlement_error"));
	}
}

Itch::~Itch()
{
	if (http_request)
	{
		http_request->queue_free();
	}
	if (s_singleton == this)
	{
		s_singleton = nullptr;
	}
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

// Delegate launch detection to Auth submodule
void Itch::detect_launch_source()
{
	Auth::get_singleton()->do_detect_launch_source();
}

// Delegate authentication methods to Auth submodule
bool Itch::is_launched_via_itch() const
{
	return Auth::get_singleton()->is_launched_via_itch();
}

bool Itch::has_api_key_present() const
{
	return Auth::get_singleton()->has_api_key_present();
}

// Module access methods
Entitlements *Itch::get_entitlements() const
{
	return Entitlements::get_singleton();
}

Core *Itch::get_core() const
{
	return Core::get_singleton();
}

Auth *Itch::get_auth() const
{
	return Auth::get_singleton();
}

User *Itch::get_user() const
{
	return User::get_singleton();
}

Games *Itch::get_games() const
{
	return Games::get_singleton();
}

// These methods are now handled by Core and Auth modules
// ensure_project_settings() -> Core::ensure_project_settings()
// get_api_key_from_settings() -> Auth::get_api_key()
// get_game_id_from_settings() -> Core::get_game_id()

// get_game_id_from_settings() removed - use get_game_id() which delegates to Core

// OAuth methods are implemented inline in godotitch.h

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
	// Use JWT endpoints; endpoint should start with '/'
	return String("https://itch.io/api/1/jwt") + endpoint;
}

void Itch::verify_download_key(const String &download_key)
{
	// Delegate to the Entitlements module
	Entitlements *entitlements = get_entitlements();
	if (entitlements)
	{
		entitlements->verify_entitlement(download_key);
	}
	else
	{
		UtilityFunctions::push_error("Entitlements module not available");
		emit_signal("verify_download_key_result", false, Dictionary());
	}
}

void Itch::get_credentials_info()
{
	// Delegate to the Auth module
	Auth *auth = get_auth();
	if (auth)
	{
		auth->get_credentials_info();
	}
	else
	{
		UtilityFunctions::push_error("Auth module not available");
		emit_signal("auth_result", false, Dictionary());
	}
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

// Utility Methods - delegate to modular architecture
void Itch::set_game_id(const String &game_id)
{
	Core::get_singleton()->set_game_id(game_id);
}

String Itch::get_game_id() const
{
	return Core::get_singleton()->get_game_id();
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

	// Also give subsystems a chance to initialize with the scene node so they can
	// create and attach their own HTTPRequest nodes if needed.
	Entitlements *ent = Entitlements::get_singleton();
	if (ent)
	{
		UtilityFunctions::print("Itch: Initializing Entitlements with scene node");
		ent->initialize_with_scene(scene_node);
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
	if (!is_verify_type)
	{
		return;
	}

	bool verified = false;
	// For itch.io download key endpoint, success usually includes a "download_key" object
	if (data.has("download_key"))
	{
		Variant dk = data["download_key"];
		// Consider presence of object as verification success
		verified = dk.get_type() == Variant::DICTIONARY || dk.get_type() == Variant::OBJECT;
	}
	// Fallback: If HTTP handled non-dict, check a generic "result"
	if (!verified && data.has("result"))
	{
		verified = true; // any result treated as success here
	}


	emit_signal("verify_download_key_result", verified, data);
}

// Entitlements signal handlers
void Itch::_on_entitlement_verified(bool success, const Dictionary &data)
{
	// Forward the entitlements signal to the facade's verify_download_key_result signal
	emit_signal("verify_download_key_result", success, data);
}

void Itch::_on_entitlement_error(const String &error_message)
{
	// Forward the entitlements error as a verify_download_key_result failure
	Dictionary error_data;
	error_data["error"] = error_message;
	emit_signal("verify_download_key_result", false, error_data);
}

// OAuth settings setters
// OAuth settings setters - delegate to Auth
void Itch::set_oauth_client_id(const String &client_id)
{
	Auth::get_singleton()->set_oauth_client_id(client_id);
}

void Itch::set_oauth_redirect_uri(const String &redirect_uri)
{
	Auth::get_singleton()->set_oauth_redirect_uri(redirect_uri);
}

void Itch::set_oauth_scope(const String &scope)
{
	Auth::get_singleton()->set_oauth_scope(scope);
}

// OAuth settings getters - delegate to Auth
String Itch::get_oauth_client_id() const
{
	return Auth::get_singleton()->get_oauth_client_id();
}

String Itch::get_oauth_redirect_uri() const
{
	return Auth::get_singleton()->get_oauth_redirect_uri();
}

String Itch::get_oauth_scope() const
{
	return Auth::get_singleton()->get_oauth_scope();
}

bool godot::Itch::is_user_authenticated() const
{
	return is_user_logged_in;
}

Dictionary godot::Itch::get_current_user() const
{
	return current_user;
}

String godot::Itch::get_current_user_name() const
{
	return current_user.get("name", "");
}

// Build OAuth authorization URL
String Itch::build_oauth_authorize_url(const String &client_id, const String &redirect_uri, const String &state) const
{
	// Delegate to Auth submodule
	return Auth::get_singleton()->build_oauth_authorize_url(client_id, redirect_uri, state);
}

// Open OAuth authorization URL in system browser
void Itch::start_oauth_authorization(const String &client_id, const String &redirect_uri, const String &state)
{
	// Delegate to Auth submodule
	Auth::get_singleton()->start_oauth_authorization(client_id, redirect_uri, state);
}

void Itch::oauth_login_success(const Dictionary &user)
{
	current_user = user;
	is_user_logged_in = true;
	emit_signal("user_logged_in", user);
}

void Itch::oauth_login_failed(const String &error)
{
	is_user_logged_in = false;
	current_user.clear();
	emit_signal("user_login_failed", error);
}

void Itch::oauth_logged_out()
{
	is_user_logged_in = false;
	current_user.clear();
	emit_signal("user_logged_out");
}