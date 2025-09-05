#include "godotitch.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/json.hpp>

using namespace godot;


void Itch::_bind_methods() {
	// Methods reading from ProjectSettings
	ClassDB::bind_method(D_METHOD("verify_user_ps", "username"), &Itch::verify_user_ps);
	ClassDB::bind_method(D_METHOD("is_game_bought_ps", "username"), &Itch::is_game_bought_ps);
	// Backward-compatible explicit variants
	ClassDB::bind_method(D_METHOD("verify_user", "api_key", "username"), &Itch::verify_user);
	ClassDB::bind_method(D_METHOD("is_game_bought", "game_id", "username", "api_key"), &Itch::is_game_bought);
	ClassDB::bind_method(D_METHOD("_on_request_completed", "result", "response_code", "headers", "body"), &Itch::_on_request_completed);
	ADD_SIGNAL(MethodInfo("itch_api_result", PropertyInfo(Variant::STRING, "request_type"), PropertyInfo(Variant::DICTIONARY, "result")));
}


Itch::Itch() {
	ensure_project_settings();
}


Itch::~Itch() {
	// Add your cleanup here.
}

// Helper function to perform HTTP GET request (simplified, blocking)
void Itch::verify_user(const String &api_key, const String &username) {
	pending_request_type = "verify_user";
	pending_username = username;
	String url = "https://itch.io/api/1/" + api_key + "/user/" + username;
	if (http_request) {
		http_request->request(url);
	}
}

void Itch::is_game_bought(const String &game_id, const String &username, const String &api_key) {
	pending_request_type = "is_game_bought";
	pending_username = username;
	pending_game_id = game_id;
	String url = "https://itch.io/api/1/" + api_key + "/game/" + game_id + "/purchases";
	if (http_request) {
		http_request->request(url);
	}
}

void Itch::_on_request_completed(int result, int response_code, const PackedStringArray &headers, const PackedByteArray &body) {
	Dictionary output;
	output["http_result"] = result;
	output["response_code"] = response_code;
	String body_str = String::utf8((const char *)body.ptr(), body.size());
	output["raw_body"] = body_str;
	Variant json_result = JSON::parse_string(body_str);
	if (json_result.get_type() != Variant::NIL) {
		output["json"] = json_result;
	}
	emit_signal("itch_api_result", pending_request_type, output);
}

void Itch::_notification(int what) {
	switch (what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!http_request) {
				http_request = memnew(HTTPRequest);
				add_child(http_request);
				http_request->connect("request_completed", Callable(this, "_on_request_completed"));
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (http_request) {
				http_request->disconnect("request_completed", Callable(this, "_on_request_completed"));
				remove_child(http_request);
				memdelete(http_request);
				http_request = nullptr;
			}
		} break;
	}
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

void Itch::verify_user_ps(const String &username) {
	String api_key = get_api_key_from_settings();
	verify_user(api_key, username);
}

void Itch::is_game_bought_ps(const String &username) {
	String api_key = get_api_key_from_settings();
	String game_id = get_game_id_from_settings();
	is_game_bought(game_id, username, api_key);
}