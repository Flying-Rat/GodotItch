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

private:
    static User* instance;
    
protected:
    static void _bind_methods();

public:
    // Singleton access
    static User* get_singleton();
    static void initialize();
    static void shutdown();
    
    // Constructor/Destructor
    User();
    ~User();
    
    // Basic user methods (minimal API)
    Dictionary get_current_user_profile();
    bool is_user_authenticated();
    Array get_user_games_as_developer();
};

#endif // USER_SUBSYSTEM_H