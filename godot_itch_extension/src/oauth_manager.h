#ifndef OAUTH_MANAGER_H
#define OAUTH_MANAGER_H

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot {

class OAuthManager {
private:
    const String SETTINGS_PREFIX = String("godot_itch/");
    const String SETTING_OAUTH_CLIENT_ID = String(SETTINGS_PREFIX) + String("oauth_client_id");
    const String SETTING_OAUTH_REDIRECT_URI = String(SETTINGS_PREFIX) + String("oauth_redirect_uri");
    const String SETTING_OAUTH_SCOPE = String(SETTINGS_PREFIX) + String("oauth_scope");

public:
    OAuthManager() = default;
    ~OAuthManager() = default;

    void ensure_settings() const;

    // Setters
    void set_client_id(const String &client_id) const;
    void set_redirect_uri(const String &redirect_uri) const;
    void set_scope(const String &scope) const;

    // Getters
    String get_client_id() const;
    String get_redirect_uri() const;
    String get_scope() const;

    // Build OAuth URL
    String build_authorize_url(const String &client_id = "", const String &redirect_uri = "", const String &state = "") const;

    // Open in browser
    void start_authorization(const String &client_id = "", const String &redirect_uri = "", const String &state = "") const;
};

} // namespace godot

#endif // OAUTH_MANAGER_H
