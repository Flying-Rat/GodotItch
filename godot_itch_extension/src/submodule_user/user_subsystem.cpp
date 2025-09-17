#include "user_subsystem.h"

// System includes
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>

using namespace godot;

// Explicit template instantiation for User CRTP
template class Subsystem<User>;

User::User() 
{
}

User::~User() 
{
}

void User::_bind_methods() 
{

}

