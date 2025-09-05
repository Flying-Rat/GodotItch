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
	}
}

void uninitialize_godotitch_module(ModuleInitializationLevel level) {
	// Nothing to do.
}