#ifndef GODOTITCH_H
#define GODOTITCH_H

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/http_request.hpp>

namespace godot {
    class Itch : public Node {
        GDCLASS(Itch, Node);

    private:
        String godotitch_version = "0.1.0";
    bool is_initialized = false;
    HTTPRequest *http_request = nullptr;
    String pending_request_type;
    String pending_username;
    String pending_game_id;

        // Project setting keys
        const String SETTINGS_PREFIX = String("godot_itch/");
        const String SETTING_API_KEY = String(SETTINGS_PREFIX) + String("api_key");
        const String SETTING_GAME_ID = String(SETTINGS_PREFIX) + String("game_id");
    void ensure_project_settings();
        String get_api_key_from_settings() const;
        String get_game_id_from_settings() const;
    // bool get_init_on_startup_from_settings() const;

        // protected:
        //     static void _bind_methods();

    public:
    static Itch* get_singleton();
        Itch();
        ~Itch();

        String get_godotitch_version() const { return godotitch_version; }

        // Itch.io API methods
    bool itchInitEx(uint32_t app_id = 0, bool embed_callbacks = false);

        // Async Itch.io API methods using ProjectSettings
        void verify_user_ps(const String& username);
        void is_game_bought_ps(const String& username);

        // Backward-compatible variants that accept explicit params
        void verify_user(const String& api_key, const String& username);
        void is_game_bought(const String& game_id, const String& username, const String& api_key);

        // Signal to emit results
        static void _bind_methods();
        void _on_request_completed(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body);

    protected:
        void _notification(int what);
    };
}

#endif