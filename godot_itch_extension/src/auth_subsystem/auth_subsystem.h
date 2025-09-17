#ifndef GODOTITCH_AUTH_H
#define GODOTITCH_AUTH_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "../core_subsystem/subsystem.h"

namespace godot
{
    /**
     * Auth - Handles authentication and launch detection
     * Uses CRTP for singleton pattern - no need for explicit static method declarations!
     */
    class Auth : public Subsystem<Auth>
    {
        GDCLASS(Auth, Subsystem<Auth>);

    private:
        bool initialized = false;

        // Token management
        String oauth_token = "";
        void save_token_to_cache();
        void load_token_from_cache();

        // Launch detection (moved from Core class)
        bool launched_via_itch = false;
        bool has_api_key = false;
        String launch_api_key = "";

        // Development API key (not stored in project settings)
        String dev_api_key = "";

        // Project setting keys
        const String SETTINGS_PREFIX = String("godot_itch/");
        const String SETTING_OAUTH_CLIENT_ID = String(SETTINGS_PREFIX) + String("oauth_client_id");
        const String SETTING_OAUTH_REDIRECT_URI = String(SETTINGS_PREFIX) + String("oauth_redirect_uri");
        const String SETTING_OAUTH_SCOPE = String(SETTINGS_PREFIX) + String("oauth_scope");

        void ensure_oauth_settings();
        String get_api_key_from_settings() const;


    protected:
        static void _bind_methods();

    public:
    Auth();
    ~Auth();

        // Instance state
        bool is_initialized() const { return initialized; }

    // Override virtual lifecycle methods from Subsystem<Auth>
        void initialize_instance() override;
        void shutdown_instance() override;

    public:
        // Launch detection API (exposed for main Itch class)
        void do_detect_launch_source();
        bool is_launched_via_itch() const;
        bool has_api_key_present() const;
        String get_launch_api_key() const;

        // OAuth configuration
        void set_oauth_client_id(const String &client_id);
        void set_oauth_redirect_uri(const String &redirect_uri);
        void set_oauth_scope(const String &scope);
        String get_oauth_client_id() const;
        String get_oauth_redirect_uri() const;
        String get_oauth_scope() const;

        // OAuth token accessors
        void set_oauth_token(const String &token);
        String get_oauth_token() const;

    // Unified bearer token (OAuth token or launch token from itch launcher)
    String get_bearer_token() const;
    bool has_bearer_token() const;

        // OAuth flow management
        String build_oauth_authorize_url(const String &client_id = "", const String &redirect_uri = "", const String &state = "") const;
        void start_oauth_authorization(const String &client_id = "", const String &redirect_uri = "", const String &state = "");
    };
}

#endif // GODOTITCH_AUTH_H