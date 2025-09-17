#ifndef DATA_CACHE_H
#define DATA_CACHE_H

#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class DataCache : public Object {
    GDCLASS(DataCache, Object);

private:
    static DataCache* instance;
    Dictionary data_cache;
    String data_path;
    String encryption_password;
    bool data_loaded;

    void _load_data();
    void _save_data();
    String _get_secure_path();

protected:
    static void _bind_methods();

public:
    static DataCache* get_singleton();

    DataCache();
    ~DataCache();

    void initialize();
    void shutdown();

    bool is_verified(const String &download_key);
    void set_verified(const String &download_key, bool verified, const Dictionary &metadata = Dictionary());
    Dictionary get_verification_data(const String &download_key);
    void clear_verification(const String &download_key);
    void clear_all_data();
};

#endif // DATA_CACHE_H
