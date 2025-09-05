// GDExtension registration for Godot 4
#include "register_types.h"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "godotitch.h"

using namespace godot;

void initialize_godotitch_module(ModuleInitializationLevel level) {
    if (level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        ClassDB::register_class<Itch>();
        // Add Project Setting for autoload
        ProjectSettings *ps = ProjectSettings::get_singleton();
        const String autoload_key = "godot_itch/auto_autoload";
        if (ps && !ps->has_setting(autoload_key)) {
            ps->set_setting(autoload_key, false);
        }
    // Note: Autoload registration must be done from an EditorPlugin (GDScript).
    // This C++ module exposes the setting only; the plugin should act on it.
    }
}

void uninitialize_godotitch_module(ModuleInitializationLevel level) {
	if (level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Engine removes singletons automatically on shutdown; nothing to do.
	}
}

extern "C" {
    // Initialization.
    GDExtensionBool GDE_EXPORT godotitch_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_godotitch_module);
        init_obj.register_terminator(uninitialize_godotitch_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}