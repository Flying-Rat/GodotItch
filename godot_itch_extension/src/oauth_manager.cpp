#include "oauth_manager.h"
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void OAuthManager::ensure_settings() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return;
	if (!ps->has_setting(SETTING_OAUTH_CLIENT_ID)) {
		ps->set_setting(SETTING_OAUTH_CLIENT_ID, "");
	}
	if (!ps->has_setting(SETTING_OAUTH_REDIRECT_URI)) {
		ps->set_setting(SETTING_OAUTH_REDIRECT_URI, "");
	}
	if (!ps->has_setting(SETTING_OAUTH_SCOPE)) {
		ps->set_setting(SETTING_OAUTH_SCOPE, "profile:me");
	}
}

void OAuthManager::set_client_id(const String &client_id) const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps) ps->set_setting(SETTING_OAUTH_CLIENT_ID, client_id);
}

void OAuthManager::set_redirect_uri(const String &redirect_uri) const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps) ps->set_setting(SETTING_OAUTH_REDIRECT_URI, redirect_uri);
}

void OAuthManager::set_scope(const String &scope) const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (ps) ps->set_setting(SETTING_OAUTH_SCOPE, scope);
}

String OAuthManager::get_client_id() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return "";
	Variant v = ps->get_setting(SETTING_OAUTH_CLIENT_ID);
	return v.get_type() == Variant::STRING ? (String)v : "";
}

String OAuthManager::get_redirect_uri() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return "";
	Variant v = ps->get_setting(SETTING_OAUTH_REDIRECT_URI);
	return v.get_type() == Variant::STRING ? (String)v : "";
}

String OAuthManager::get_scope() const {
	ProjectSettings *ps = ProjectSettings::get_singleton();
	if (!ps) return "profile:me";
	Variant v = ps->get_setting(SETTING_OAUTH_SCOPE);
	String s = v.get_type() == Variant::STRING ? (String)v : "profile:me";
	if (s != "profile:me") s = "profile:me";
	return s;
}

String OAuthManager::build_authorize_url(const String &client_id, const String &redirect_uri, const String &state) const {
	String cid = client_id.is_empty() ? get_client_id() : client_id;
	String ruri = redirect_uri.is_empty() ? get_redirect_uri() : redirect_uri;
	String scope = get_scope();

	if (cid.is_empty() || ruri.is_empty()) {
		UtilityFunctions::push_error("OAuth client_id and redirect_uri must be set (either via parameters or project settings).");
		return "";
	}

	String cid_enc = cid.uri_encode();
	String ruri_enc = ruri.uri_encode();
	String scope_enc = scope.uri_encode();
	String url = "https://itch.io/user/oauth?client_id=" + cid_enc + "&scope=" + scope_enc + "&redirect_uri=" + ruri_enc;
	if (!state.is_empty()) {
		url += "&state=" + state.uri_encode();
	}
	return url;
}

void OAuthManager::start_authorization(const String &client_id, const String &redirect_uri, const String &state) const {
	String url = build_authorize_url(client_id, redirect_uri, state);
	if (url.is_empty()) return;
	OS *os = OS::get_singleton();
	if (os) {
		bool ok = os->shell_open(url) == Error::OK;
		if (!ok) {
			UtilityFunctions::push_error("Failed to open OAuth authorization URL in browser.");
		}
	}
}
