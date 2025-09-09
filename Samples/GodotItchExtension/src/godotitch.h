#ifndef GODOTITCH_H
#define GODOTITCH_H

// Include INT types header
#include <inttypes.h>

// Include Godot-CPP headers for GDExtensions
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/callable.hpp>

namespace godot {
    class Itch : public Object {
        GDCLASS(Itch, Object);

    private:
        String godotitch_version = "0.1.0";
        bool is_initialized = false;
        HTTPRequest* http_request = nullptr;
        String pending_request_type;
        Dictionary pending_request_data;

        // Project setting keys
        const String SETTINGS_PREFIX = String("godot_itch/");
        const String SETTING_API_KEY = String(SETTINGS_PREFIX) + String("api_key");
        const String SETTING_GAME_ID = String(SETTINGS_PREFIX) + String("game_id");

        void ensure_project_settings();
        String get_api_key_from_settings() const;
        String get_game_id_from_settings() const;
        void _setup_http_request();
        String _build_api_url(const String& endpoint) const;

    protected:
        static void _bind_methods();

    public:
        static Itch* get_singleton();
        Itch();
        ~Itch();

        String get_godotitch_version() const { return godotitch_version; }

        // Itch.io API methods
        bool itchInitEx(uint32_t app_id = 0, bool embed_callbacks = false);

        // Async API methods (use signals for results)
        void get_me();
        void get_my_games();
        void get_game_purchases(const String& game_id = "");
        void get_game_uploads(const String& game_id = "");
        void get_download_key(const String& download_key, const String& game_id = "");

        // Utility methods
        void set_api_key(const String& api_key);
        void set_game_id(const String& game_id);
        String get_api_key() const;
        String get_game_id() const;

        // Scene management
        void initialize_with_scene(Node* scene_node);

        // HTTP callback
        void _on_request_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body);
        // Test helper: perform a plain HTTP request to example.com
        void test_request_http();
        // Internal deferred request helper
        void _perform_request(const String& url, const PackedStringArray& headers);
        // Post-request diagnostic check
        void post_request_check();
    };
}

#endif