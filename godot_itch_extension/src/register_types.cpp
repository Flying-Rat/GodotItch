#include "register_types.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>

#include "godotitch.h"
#include "core/core.h"
#include "core/persistent/itch_data_cache.h"
#include "submodule_auth/itch_auth.h"
#include "submodule_entitlements/entitlements.h"

using namespace godot;

static Itch* ItchPtr;

void initialize_godotitch_module(ModuleInitializationLevel level) {
    if(level == MODULE_INITIALIZATION_LEVEL_CORE){
        // Register Core first
        ClassDB::register_class<Core>();
        
        // Register data cache
        ClassDB::register_class<ItchDataCache>();
        
        // Register auth submodule  
        ClassDB::register_class<ItchAuth>();
        
        // Register entitlements module
        ClassDB::register_class<Entitlements>();
        
        // Register main Itch class
        ClassDB::register_class<Itch>();
        
        // Initialize singletons in dependency order
        Core::get_singleton()->initialize();
        ItchAuth::get_singleton()->initialize();
        Entitlements::get_singleton()->initialize();
        
        // Register the singleton instance
        ItchPtr = memnew(Itch);
        Engine::get_singleton()->register_singleton("Itch", ItchPtr);
    }
}

void uninitialize_godotitch_module(ModuleInitializationLevel level) {
    if(level == MODULE_INITIALIZATION_LEVEL_CORE){
        Engine::get_singleton()->unregister_singleton("Itch");
        memdelete(ItchPtr);
        
        // Shutdown singletons in reverse order
        Entitlements::get_singleton()->shutdown();
        ItchAuth::get_singleton()->shutdown();
        Core::get_singleton()->shutdown();
    }
}

extern "C" {
    // Initialization
    GDExtensionBool GDE_EXPORT godotitch_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
        
        init_obj.register_initializer(initialize_godotitch_module);
        init_obj.register_terminator(uninitialize_godotitch_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_CORE);
        
        return init_obj.init();
    }
}