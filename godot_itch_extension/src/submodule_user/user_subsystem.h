#ifndef USER_SUBSYSTEM_H
#define USER_SUBSYSTEM_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "../core/submodule.h"

using namespace godot;

/**
 * User - Minimal stub for user management
 * Uses CRTP for singleton pattern - no need for explicit static method declarations!
 */
class User : public Submodule<User> {
    GDCLASS(User, Submodule<User>);

protected:
    static void _bind_methods();

public:
    // Constructor/Destructor
    User();
    ~User();
};

#endif // USER_SUBSYSTEM_H