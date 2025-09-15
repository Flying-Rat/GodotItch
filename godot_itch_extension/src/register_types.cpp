// GDExtension registration for Godot 4
#include "register_types.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "core/core.h"
#include "godotitch.h"
#include "core/persistent/itch_data_cache.h"

using namespace godot;

static Itch *ItchPtr = nullptr;


void initialize_godotitch_module(ModuleInitializationLevel level) {
    	if(level == MODULE_INITIALIZATION_LEVEL_CORE){
		// Register Core first
		ClassDB::register_class<Core>();
		
		// Register ItchDataCache
		ClassDB::register_class<ItchDataCache>();
		
		// Register main Itch class
		ClassDB::register_class<Itch>();
		
		// Initialize Core singleton first (this also initializes ItchDataStore)
		Core::get_singleton()->initialize();
		
		ItchPtr = memnew(Itch);
		Engine::get_singleton()->register_singleton("Itch", Itch::get_singleton());

		// Note: GLOBAL_DEF_BASIC is not available in GDExtensions
		// Settings should be managed through other means
		// For now, we'll initialize with default values
		uint32_t app_id = 0;
		bool auto_init = false;
		bool embed_callbacks = false;

		if (auto_init){
			// Prevent intializing Itch from the editor itself
			if (Engine::get_singleton()->is_editor_hint()) {
				return;
			}
			Itch::get_singleton()->itchInitEx(app_id, embed_callbacks);
		}
	}
}

void uninitialize_godotitch_module(ModuleInitializationLevel level) {
	if(level == MODULE_INITIALIZATION_LEVEL_CORE){
		Engine::get_singleton()->unregister_singleton("Itch");
		memdelete(ItchPtr);
		
		// Shutdown Core singleton (this also shuts down ItchDataStore)
		Core::get_singleton()->shutdown();
	}
}

extern "C" {
    // Initialization.
    GDExtensionBool GDE_EXPORT godotitch_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_godotitch_module);
        init_obj.register_terminator(uninitialize_godotitch_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_CORE);

        return init_obj.init();
    }
}