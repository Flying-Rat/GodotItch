#ifndef GAMES_SUBSYSTEM_H
#define GAMES_SUBSYSTEM_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "../core_subsystem/subsystem.h"

using namespace godot;

/**
 * Games - Minimal stub for game management
 * Uses CRTP for singleton pattern - no need for explicit static method declarations!
 */
class Games : public Subsystem<Games> {
    GDCLASS(Games, Subsystem<Games>);

protected:
    static void _bind_methods();

public:
    // Constructor/Destructor
    Games();
    ~Games();

    // TODO(jakub.hubacek): suggest user-friendly game related methods here
    // e.g. get_game_info, search_games, check_game_ownership, etc.
};

#endif // GAMES_SUBSYSTEM_H