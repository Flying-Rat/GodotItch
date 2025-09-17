#include "data_cache.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/file_access.hpp>

using namespace godot;

DataCache* DataCache::instance = nullptr;

void DataCache::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("is_verified", "download_key"), &DataCache::is_verified);
    ClassDB::bind_method(D_METHOD("set_verified", "download_key", "verified", "metadata"),
                         &DataCache::set_verified, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_verification_data", "download_key"), &DataCache::get_verification_data);
    ClassDB::bind_method(D_METHOD("clear_verification", "download_key"), &DataCache::clear_verification);
    ClassDB::bind_method(D_METHOD("clear_all_data"), &DataCache::clear_all_data);
}

DataCache* DataCache::get_singleton()
{
    if (!instance) {
        instance = memnew(DataCache);
    }
    return instance;
}

DataCache::DataCache() {
    data_loaded = false;
    data_path = _get_secure_path();
    // Use a secure password for encryption - you can customize this
    encryption_password = "DataCache_2025_SecureKey";
}

DataCache::~DataCache() {
    if (instance == this) {
        instance = nullptr;
    }
}

void DataCache::initialize() {
    _load_data();
}

void DataCache::shutdown() {
    _save_data();
}

String DataCache::_get_secure_path() {
    // Use user:// for secure, per-user data storage
    // This follows Godot's recommended pattern for user data
    return "user://itch_verification_cache.dat";
}

void DataCache::_load_data() 
{
    if (data_loaded) 
    {
        return;
    }

    // Try to load encrypted JSON data
    Ref<FileAccess> file = FileAccess::open_encrypted_with_pass(data_path, FileAccess::READ, encryption_password);
    if (file.is_valid()) 
    {
        // Read the encrypted content
        String json_content = file->get_as_text();
        file->close();
        
        // Parse the JSON content
        Ref<JSON> json;
        json.instantiate();
        Error err = json->parse(json_content);
        
        if (err == OK) 
        {
            data_cache = json->get_data();
        } else 
        {
            UtilityFunctions::push_warning("Failed to parse JSON data, starting fresh");
            data_cache = Dictionary();
        }
    } else 
    {
        // File doesn't exist or can't be opened - this is normal for first run
        UtilityFunctions::print("No encrypted JSON data file found, starting fresh");
        data_cache = Dictionary();
    }
    
    data_loaded = true;
}

void DataCache::_save_data() 
{
    if (!data_loaded) {
        return; // Nothing to save if we haven't loaded anything
    }

    // Convert data to JSON string
    Ref<JSON> json;
    json.instantiate();
    String json_content = json->stringify(data_cache, "    ", false, false);
    
    // Save encrypted
    Ref<FileAccess> file = FileAccess::open_encrypted_with_pass(data_path, FileAccess::WRITE, encryption_password);
    if (file.is_valid()) {
        file->store_string(json_content);
        file->close();
    } else {
        UtilityFunctions::push_error("Failed to save encrypted JSON file");
    }
}

bool DataCache::is_verified(const String &download_key) 
{
    _load_data();
    
    Dictionary verifications = data_cache.get("verifications", Dictionary());
    return verifications.get(download_key, false);
}

void DataCache::set_verified(const String &download_key, bool verified, const Dictionary &metadata) 
{
    _load_data();
    
    // Update verifications
    Dictionary verifications = data_cache.get("verifications", Dictionary());
    verifications[download_key] = verified;
    data_cache["verifications"] = verifications;
    
    if (verified && !metadata.is_empty()) {
        // Store metadata with timestamp
        Dictionary metadata_store = data_cache.get("metadata", Dictionary());
        Dictionary entry_metadata = metadata.duplicate();
        entry_metadata["timestamp"] = Time::get_singleton()->get_unix_time_from_system();
        metadata_store[download_key] = entry_metadata;
        data_cache["metadata"] = metadata_store;
    } else if (!verified) {
        // Remove metadata when unverifying
        Dictionary metadata_store = data_cache.get("metadata", Dictionary());
        if (metadata_store.has(download_key)) {
            metadata_store.erase(download_key);
            data_cache["metadata"] = metadata_store;
        }
    }
    
    _save_data();
}

Dictionary DataCache::get_verification_data(const String &download_key) {
    _load_data();
    
    Dictionary metadata_store = data_cache.get("metadata", Dictionary());
    return metadata_store.get(download_key, Dictionary());
}

void DataCache::clear_verification(const String &download_key) {
    _load_data();
    
    // Remove from verifications
    Dictionary verifications = data_cache.get("verifications", Dictionary());
    if (verifications.has(download_key)) {
        verifications.erase(download_key);
        data_cache["verifications"] = verifications;
    }
    
    // Remove from metadata
    Dictionary metadata_store = data_cache.get("metadata", Dictionary());
    if (metadata_store.has(download_key)) {
        metadata_store.erase(download_key);
        data_cache["metadata"] = metadata_store;
    }
    
    _save_data();
}

void DataCache::clear_all_data() {
    _load_data();
    data_cache.clear();
    _save_data();
}
