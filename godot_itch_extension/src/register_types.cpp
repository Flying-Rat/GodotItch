#include "register_types.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>

#include "godotitch.h"
#include "core_subsystem/core_subsystem.h"
#include "core_subsystem/persistent/data_cache.h"
#include "auth_subsystem/auth_subsystem.h"
#include "entitlements_subsystem/entitlements_subsystem.h"
#include "user_subsystem/user_subsystem.h"
#include "games_subsystem/games_subsystem.h"

using namespace godot;

static Itch* ItchPtr;

void initialize_godotitch_module(ModuleInitializationLevel level)
{
    if (level == MODULE_INITIALIZATION_LEVEL_CORE) {
        // Register Core first
        ClassDB::register_class<Core>();

        // Register submodules in dependency order
        ClassDB::register_class<DataCache>();
        ClassDB::register_class<Auth>();
        ClassDB::register_class<Entitlements>();
        ClassDB::register_class<User>();
        ClassDB::register_class<Games>();
        ClassDB::register_class<Itch>();
        // Phase 1: Create singletons in dependency order
        Core::create_singleton();
        Auth::create_singleton();
        Entitlements::create_singleton();
        User::create_singleton();
        Games::create_singleton();

        // Phase 2: Initialize singletons (calls virtual initialize_instance() on each)
        Core::initialize();
        Auth::initialize();
        Entitlements::initialize();
        User::initialize();
        Games::initialize();

        // Additional scene-based initialization for some modules
        Entitlements::get_singleton()->initialize_with_scene(nullptr);

        // Register the singleton instance
        ItchPtr = memnew(Itch);
        Engine::get_singleton()->register_singleton("Itch", ItchPtr);
    }
}

void uninitialize_godotitch_module(ModuleInitializationLevel level)
{
    if (level == MODULE_INITIALIZATION_LEVEL_CORE) 
    {
        Engine::get_singleton()->unregister_singleton("Itch");
        memdelete(ItchPtr);

        // Shutdown singletons in reverse order (calls shutdown_impl() then deletes)
        Games::shutdown();
        User::shutdown();
        Entitlements::shutdown();
        Auth::shutdown();
        Core::shutdown();
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