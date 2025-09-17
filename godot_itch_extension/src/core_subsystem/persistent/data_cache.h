#ifndef DATA_CACHE_H
#define DATA_CACHE_H

// DataCache removed: keep a minimal stub so remaining references compile.
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

class DataCache : public Object {
    GDCLASS(DataCache, Object);
public:
    static DataCache* get_singleton() { return nullptr; }
    void initialize() {}
    void shutdown() {}
    bool is_verified(const String &download_key) { return false; }
    void set_verified(const String &download_key, bool verified, const Dictionary &metadata = Dictionary()) {}
    Dictionary get_verification_data(const String &download_key) { return Dictionary(); }
    void clear_verification(const String &download_key) {}
    void clear_all_data() {}
};

#endif // DATA_CACHE_H
