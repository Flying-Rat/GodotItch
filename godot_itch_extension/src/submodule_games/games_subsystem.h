#ifndef GAMES_SUBSYSTEM_H
#define GAMES_SUBSYSTEM_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

/**
 * Games - Minimal stub for game management
 */
class Games : public Object {
    GDCLASS(Games, Object);

protected:
    static void _bind_methods();

public:
    // Lifecycle via SubsystemTemplate<Games>
    static Games* get_singleton();
    static void initialize();
    static void shutdown();

    // Constructor/Destructor
    Games();
    ~Games();

    // TODO(jakub.hubacek): suggest user-friendly game related methods here
    // e.g. get_game_info, search_games, check_game_ownership, etc.
};

#endif // GAMES_SUBSYSTEM_H