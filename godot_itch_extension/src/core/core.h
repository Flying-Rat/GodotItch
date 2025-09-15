#ifndef GODOTITCH_CORE_H
#define GODOTITCH_CORE_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include "persistent/itch_data_cache.h"

namespace godot {
    class Core : public Object {
        GDCLASS(Core, Object);
        
    private:
        static Core* instance;
        bool initialized = false;
        
        // Launch detection (moved from Itch class)
        bool launched_via_itch = false;
        bool has_api_key = false;
        String launch_api_key = "";
        
        ItchDataCache* persistent_cache = nullptr;
        
        // Project setting keys
        const String SETTINGS_PREFIX = String("godot_itch/");
        const String SETTING_API_KEY = String(SETTINGS_PREFIX) + String("api_key");
        const String SETTING_GAME_ID = String(SETTINGS_PREFIX) + String("game_id");
        const String SETTING_OAUTH_CLIENT_ID = String(SETTINGS_PREFIX) + String("oauth_client_id");
        const String SETTING_OAUTH_REDIRECT_URI = String(SETTINGS_PREFIX) + String("oauth_redirect_uri");
        const String SETTING_OAUTH_SCOPE = String(SETTINGS_PREFIX) + String("oauth_scope");
        
        void detect_launch_source();
        void ensure_project_settings();
        String get_api_key_from_settings() const;
        String get_game_id_from_settings() const;
        
    protected:
        static void _bind_methods();
        
    public:
        static Core* get_singleton();
        
        Core();
        ~Core();
        
        // Initialization
        bool initialize();
        void shutdown();
        bool is_initialized() const { return initialized; }
        
        // Launch detection (preserve existing API)
        bool is_launched_via_itch() const;
        bool has_api_key_present() const;
        String get_launch_api_key() const;
        
        // Configuration
        void set_api_key(const String& api_key);
        String get_api_key() const;
        void set_game_id(const String& game_id);
        String get_game_id() const;
        
        // OAuth configuration
        void set_oauth_client_id(const String& client_id);
        void set_oauth_redirect_uri(const String& redirect_uri);
        void set_oauth_scope(const String& scope);
        String get_oauth_client_id() const;
        String get_oauth_redirect_uri() const;
        String get_oauth_scope() const;
        
        // Persistent cache access
        ItchDataCache* get_persistent_cache() { return persistent_cache; }
    };
}

#endif // GODOTITCH_CORE_H