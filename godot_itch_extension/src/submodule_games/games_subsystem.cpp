#include "games_subsystem.h"

// System includes
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>

// Local includes
#include "../core/submodule.h"
using namespace godot;

// Explicit template instantiation for Games CRTP
template class Submodule<Games>;

Games::Games()
{
}

Games::~Games() 
{
}

void Games::_bind_methods() 
{
}