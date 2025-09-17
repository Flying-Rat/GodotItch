#ifndef GODOTITCH_H
#define GODOTITCH_H

// System includes
#include <inttypes.h>

// Godot-CPP includes
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

// Local includes
#include "core_subsystem/persistent/itch_data_cache.h"
#include "core_subsystem/core_subsystem.h"
#include "auth_subsystem/auth_subsystem.h"
#include "entitlements_subsystem/entitlements_subsystem.h"
#include "submodule_user/user_subsystem.h"
#include "submodule_games/games_subsystem.h"

namespace godot
{
    class Itch : public Object
    {
        GDCLASS(Itch, Object);

    private:
        String godotitch_version = "1.1.0";
        bool is_initialized = false;
        HTTPRequest *http_request = nullptr;
        String pending_request_type;
        Dictionary pending_request_data;
        ItchDataCache* data_cache = nullptr;
        ItchAuth* auth = nullptr;
        
        // OAuth configuration
        String oauth_client_id;
        String oauth_redirect_uri;
        String oauth_scope;
        
        // User state
        Dictionary current_user;
        bool is_user_logged_in = false;
        
        void _setup_http_request();
        String _build_api_url(const String& endpoint) const;

    protected:
        static void _bind_methods();

    public:
        static Itch *get_singleton();
        Itch();
        ~Itch();

        // Launch detection methods
        void detect_launch_source();
        bool is_launched_via_itch() const;
        bool has_api_key_present() const;

        String get_godotitch_version() const { return godotitch_version; }

        // Module access - expose all subsystems
        Entitlements* get_entitlements() const;
        Core* get_core() const;
        ItchAuth* get_auth() const;
        User* get_user() const;
        Games* get_games() const;
    // Assets subsystem removed

        // Itch.io API methods
        bool itchInitEx(uint32_t app_id = 0, bool embed_callbacks = false);

        // Async API methods (use signals for results)
        void get_me();
        void get_my_games();
        void get_game_purchases(const String &game_id = "");
        void get_game_uploads(const String &game_id = "");
        void get_download_key(const String &download_key, const String &game_id = "");

        // Itch.io API wrappers
        void verify_download_key(const String &download_key);

        // Utility methods
        void set_api_key(const String &api_key);
        void set_game_id(const String &game_id);
        String get_api_key() const;
        String get_game_id() const;

        // OAuth helpers
        void set_oauth_client_id(const String &client_id);
        void set_oauth_redirect_uri(const String &redirect_uri);
        void set_oauth_scope(const String &scope);
        String get_oauth_client_id() const;
        String get_oauth_redirect_uri() const;
        String get_oauth_scope() const;

        // Build the authorization URL for itch.io OAuth
        String build_oauth_authorize_url(const String &client_id = "", const String &redirect_uri = "", const String &state = "") const;
        // Open the authorization URL in the system browser
        void start_oauth_authorization(const String &client_id = "", const String &redirect_uri = "", const String &state = "");

        // Scene management
        void initialize_with_scene(Node *scene_node);

        // HTTP callback
        void _on_request_completed(int result, int response_code, const PackedStringArray &headers, const PackedByteArray &body);
        // Hook for own api_response signal
        void _on_api_response(const String& endpoint, const Dictionary& data);
        // Entitlements signal handlers
        void _on_entitlement_verified(bool success, const Dictionary& data);
        void _on_entitlement_error(const String& error_message);
        // Test helper: perform a plain HTTP request to example.com
        void test_request_http();
        // Internal deferred request helper
        void _perform_request(const String &url, const PackedStringArray &headers);
        // Post-request diagnostic check
        void post_request_check();

        // Auth helpers to emit signals
        void emit_user_logged_in(const Dictionary &user_dict) { emit_signal("user_logged_in", user_dict); }

        // OAuth hooks to be called by integration
        void oauth_login_success(const Dictionary &user);
        void oauth_login_failed(const String &error);
        void oauth_logged_out();
    };
}

#endif