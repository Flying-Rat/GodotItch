#ifndef GODOTITCH_CORE_H
#define GODOTITCH_CORE_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include "subsystem.h"

namespace godot {
    /**
     * Core - Central configuration and initialization
     * Uses CRTP for singleton pattern - no need for explicit static method declarations!
     */
    class Core : public Subsystem<Core> {
        GDCLASS(Core, Subsystem<Core>);
        
    private:
        bool initialized = false;

        // Project setting keys
        const String SETTINGS_PREFIX = String("godot_itch/");
        const String SETTING_GAME_ID = String(SETTINGS_PREFIX) + String("game_id");
        const String SETTING_API_KEY = String(SETTINGS_PREFIX) + String("api_key");
        
        void ensure_project_settings();
        String get_game_id_from_settings() const;
        
    protected:
        static void _bind_methods();
        
    public:
        Core();
        ~Core();
        
        // Instance state 
        bool is_initialized() const { return initialized; }

        // Override virtual lifecycle methods from Subsystem<Core>
        void initialize_instance() override;
        void shutdown_instance() override;
        
    public:
        // Configuration (exposed for main Itch class)
        void set_game_id(const String& game_id);
        String get_game_id() const;
        
    };
}

#endif // GODOTITCH_CORE_H