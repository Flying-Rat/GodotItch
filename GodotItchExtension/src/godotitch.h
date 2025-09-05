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

namespace godot {
    class Itch : public Object {
        GDCLASS(Itch, Object);

    private:
        String godotitch_version = "0.1.0";
        bool is_initialized = false;

        // Project setting keys
        const String SETTINGS_PREFIX = String("godot_itch/");
        const String SETTING_API_KEY = String(SETTINGS_PREFIX) + String("api_key");
        const String SETTING_GAME_ID = String(SETTINGS_PREFIX) + String("game_id");
        void ensure_project_settings();
        String get_api_key_from_settings() const;
        String get_game_id_from_settings() const;

    protected:
        static void _bind_methods();

    public:
        static Itch* get_singleton();
        Itch();
        ~Itch();

        String get_godotitch_version() const { return godotitch_version; }

        // Itch.io API methods
        bool itchInitEx(uint32_t app_id = 0, bool embed_callbacks = false);
    };
}

#endif