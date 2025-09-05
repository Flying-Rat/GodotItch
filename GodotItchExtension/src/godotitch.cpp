#include "godotitch.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

static Itch *s_singleton = nullptr;


void Itch::_bind_methods() {
	// Methods reading from ProjectSettings
	// ClassDB::bind_method(D_METHOD("verify_user_ps", "username"), &Itch::verify_user_ps);
	// ClassDB::bind_method(D_METHOD("is_game_bought_ps", "username"), &Itch::is_game_bought_ps);
	// // Backward-compatible explicit variants
	// ClassDB::bind_method(D_METHOD("verify_user", "api_key", "username"), &Itch::verify_user);
	// ClassDB::bind_method(D_METHOD("is_game_bought", "game_id", "username", "api_key"), &Itch::is_game_bought);
	// ClassDB::bind_method(D_METHOD("_on_request_completed", "result", "response_code", "headers", "body"), &Itch::_on_request_completed);
	// ADD_SIGNAL(MethodInfo("itch_api_result", PropertyInfo(Variant::STRING, "request_type"), PropertyInfo(Variant::DICTIONARY, "result")));
}


Itch::Itch() {
	ensure_project_settings();
	s_singleton = this;
}


Itch::~Itch() {
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