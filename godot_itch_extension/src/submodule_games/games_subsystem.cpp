#include "games_subsystem.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include "../core/subsystem_template.h"
#include "../core/stub_helpers.h"

using namespace godot;

// Forward singleton methods to SubsystemTemplate<Games>
Games* Games::get_singleton() 
{ 
    return SubsystemTemplate<Games>::get_singleton(); 
}
void Games::initialize() 
{ 
    SubsystemTemplate<Games>::initialize(); 
}
void Games::shutdown() 
{ 
    SubsystemTemplate<Games>::shutdown(); 
}

Games::Games() : Object() 
{
}

Games::~Games() 
{
}

void Games::_bind_methods() 
{
}