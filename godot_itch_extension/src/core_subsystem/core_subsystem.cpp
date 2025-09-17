#include "core_subsystem.h"
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Explicit template instantiation for Core CRTP
template class Subsystem<Core>;

void Core::_bind_methods()
{
    // Initialization
    ClassDB::bind_method(D_METHOD("is_initialized"), &Core::is_initialized);

    // Configuration
    ClassDB::bind_method(D_METHOD("set_game_id", "game_id"), &Core::set_game_id);
    ClassDB::bind_method(D_METHOD("get_game_id"), &Core::get_game_id);
}

Core::Core() 
{

}

Core::~Core() 
{
    // Destructor - cleanup handled by shutdown_impl()
}

// Override virtual initialization from Subsystem<Core>
void Core::initialize_instance() 
{
    if (initialized) 
    {
        return;
    }
    
    UtilityFunctions::print("Core: Initializing...");
    
    // Initialize project settings first
    ensure_project_settings();
    
    initialized = true;
    UtilityFunctions::print("Core: Initialization complete");
}

// Override virtual shutdown from Subsystem<Core>  
void Core::shutdown_instance() 
{
    if (!initialized) 
    {
        return;
    }
    
    UtilityFunctions::print("Core: Shutting down...");
    
    initialized = false;
    UtilityFunctions::print("Core: Shutdown complete");
}

void Core::ensure_project_settings() 
{
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (!ps) 
    {
        return;
    }
    
    if (!ps->has_setting(SETTING_GAME_ID)) 
    {
        ps->set_setting(SETTING_GAME_ID, "");
    }
}

String Core::get_game_id_from_settings() const 
{
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (!ps) 
    {
        return "";
    }
    Variant v = ps->get_setting(SETTING_GAME_ID);
    if (v.get_type() == Variant::STRING) 
    {
        return v;
    }
    return "";
}

// Configuration methods
void Core::set_game_id(const String& game_id) 
{
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (ps) 
    {
        ps->set_setting(SETTING_GAME_ID, game_id);
    }
}

String Core::get_game_id() const 
{
    return get_game_id_from_settings();
}