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
}

Core::Core()
{

}

Core::~Core()
{

}

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
    // empty for now - no special settings to ensure
}