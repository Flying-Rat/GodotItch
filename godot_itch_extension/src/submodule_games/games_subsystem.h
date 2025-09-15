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

private:
    static Games* instance;
    
protected:
    static void _bind_methods();

public:
    // Singleton access
    static Games* get_singleton();
    static void initialize();
    static void shutdown();
    
    // Constructor/Destructor
    Games();
    ~Games();
    
    // Basic game methods (minimal API)
    Dictionary get_game_info(int game_id);
    Array search_games(const String& query, const Dictionary& options = Dictionary());
    bool check_game_ownership(int game_id);
};

#endif // GAMES_SUBSYSTEM_H