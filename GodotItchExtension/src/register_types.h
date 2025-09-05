#ifndef GODOTITCH_REGISTER_TYPES_H
#define GODOTITCH_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_godotitch_module(ModuleInitializationLevel level);
void uninitialize_godotitch_module(ModuleInitializationLevel level);

#endif