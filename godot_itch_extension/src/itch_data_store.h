#ifndef ITCH_DATA_STORE_H
#define ITCH_DATA_STORE_H

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class ItchDataStore : public Object {
    GDCLASS(ItchDataStore, Object);

private:
    static ItchDataStore* instance;
    Dictionary data_store;
    String data_path;
    String encryption_password;
    bool data_loaded;

    void _load_data();
    void _save_data();
    String _get_secure_path();

protected:
    static void _bind_methods();

public:
    static ItchDataStore* get_singleton();
    
    ItchDataStore();
    ~ItchDataStore();

    void initialize();
    void shutdown();

    bool is_verified(const String &download_key);
    void set_verified(const String &download_key, bool verified, const Dictionary &metadata = Dictionary());
    Dictionary get_verification_data(const String &download_key);
    void clear_verification(const String &download_key);
    void clear_all_data();
};

#endif // ITCH_DATA_STORE_H
