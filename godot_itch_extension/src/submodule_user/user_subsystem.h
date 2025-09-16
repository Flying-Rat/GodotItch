#ifndef USER_SUBSYSTEM_H
#define USER_SUBSYSTEM_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

/**
 * User - Minimal stub for user management
 */
class User : public Object {
    GDCLASS(User, Object);

protected:
    static void _bind_methods();

public:
    // Lifecycle via SubsystemTemplate<User>
    static User* get_singleton();
    static void initialize();
    static void shutdown();

    // Constructor/Destructor
    User();
    ~User();
};

#endif // USER_SUBSYSTEM_H