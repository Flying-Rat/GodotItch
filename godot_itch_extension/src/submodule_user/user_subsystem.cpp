#include "user_subsystem.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include "../core/subsystem_template.h"
#include "../core/stub_helpers.h"

using namespace godot;

// Forward singleton methods to SubsystemTemplate<User>
User* User::get_singleton() 
{ 
    return SubsystemTemplate<User>::get_singleton(); 
}
void User::initialize() 
{ 
    SubsystemTemplate<User>::initialize(); 
}
void User::shutdown() 
{ 
    SubsystemTemplate<User>::shutdown(); 
}

User::User() : Object() 
{
}

User::~User() 
{
}

void User::_bind_methods() 
{

}

