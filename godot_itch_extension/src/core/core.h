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
        
        ItchDataCache* persistent_cache = nullptr;
        
        // Project setting keys
        const String SETTINGS_PREFIX = String("godot_itch/");
        const String SETTING_GAME_ID = String(SETTINGS_PREFIX) + String("game_id");
        const String SETTING_API_KEY = String(SETTINGS_PREFIX) + String("api_key");
        
        void ensure_project_settings();
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
        
        // Configuration
        void set_game_id(const String& game_id);
        String get_game_id() const;
        
        // Persistent cache access
        ItchDataCache* get_persistent_cache() { return persistent_cache; }
    };
}

#endif // GODOTITCH_CORE_H