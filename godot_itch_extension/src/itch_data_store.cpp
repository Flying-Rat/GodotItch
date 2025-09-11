#include "itch_data_store.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/file_access.hpp>

using namespace godot;

ItchDataStore* ItchDataStore::instance = nullptr;

void ItchDataStore::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_verified", "download_key"), &ItchDataStore::is_verified);
    ClassDB::bind_method(D_METHOD("set_verified", "download_key", "verified", "metadata"), &ItchDataStore::set_verified, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_verification_data", "download_key"), &ItchDataStore::get_verification_data);
    ClassDB::bind_method(D_METHOD("clear_verification", "download_key"), &ItchDataStore::clear_verification);
    ClassDB::bind_method(D_METHOD("clear_all_data"), &ItchDataStore::clear_all_data);
}

ItchDataStore* ItchDataStore::get_singleton() {
    if (!instance) {
        instance = memnew(ItchDataStore);
    }
    return instance;
}

ItchDataStore::ItchDataStore() {
    data_loaded = false;
    data_path = _get_secure_path();
    // Use a secure password for encryption - you can customize this
    encryption_password = "ItchDataStore_2025_SecureKey";
}

ItchDataStore::~ItchDataStore() {
    if (instance == this) {
        instance = nullptr;
    }
}

void ItchDataStore::initialize() {
    _load_data();
}

void ItchDataStore::shutdown() {
    _save_data();
}

String ItchDataStore::_get_secure_path() {
    // Use user:// for secure, per-user data storage
    // This follows Godot's recommended pattern for user data
    return "user://itch_verification_data.dat";
}

void ItchDataStore::_load_data() {
    if (data_loaded) {
        return;
    }

    // Try to load encrypted JSON data
    Ref<FileAccess> file = FileAccess::open_encrypted_with_pass(data_path, FileAccess::READ, encryption_password);
    if (file.is_valid()) {
        // Read the encrypted content
        String json_content = file->get_as_text();
        file->close();
        
        // Parse the JSON content
        Ref<JSON> json;
        json.instantiate();
        Error err = json->parse(json_content);
        
        if (err == OK) {
            data_store = json->get_data();
        } else {
            UtilityFunctions::push_warning("Failed to parse JSON data, starting fresh");
            data_store = Dictionary();
        }
    } else {
        // File doesn't exist or can't be opened - this is normal for first run
        UtilityFunctions::print("No encrypted JSON data file found, starting fresh");
        data_store = Dictionary();
    }
    
    data_loaded = true;
}

void ItchDataStore::_save_data() {
    if (!data_loaded) {
        return; // Nothing to save if we haven't loaded anything
    }

    // Convert data to JSON string
    Ref<JSON> json;
    json.instantiate();
    String json_content = json->stringify(data_store, "    ", false, false);
    
    // Save encrypted
    Ref<FileAccess> file = FileAccess::open_encrypted_with_pass(data_path, FileAccess::WRITE, encryption_password);
    if (file.is_valid()) {
        file->store_string(json_content);
        file->close();
    } else {
        UtilityFunctions::push_error("Failed to save encrypted JSON file");
    }
}

bool ItchDataStore::is_verified(const String &download_key) {
    _load_data();
    
    Dictionary verifications = data_store.get("verifications", Dictionary());
    return verifications.get(download_key, false);
}

void ItchDataStore::set_verified(const String &download_key, bool verified, const Dictionary &metadata) {
    _load_data();
    
    // Update verifications
    Dictionary verifications = data_store.get("verifications", Dictionary());
    verifications[download_key] = verified;
    data_store["verifications"] = verifications;
    
    if (verified && !metadata.is_empty()) {
        // Store metadata with timestamp
        Dictionary metadata_store = data_store.get("metadata", Dictionary());
        Dictionary entry_metadata = metadata.duplicate();
        entry_metadata["timestamp"] = Time::get_singleton()->get_unix_time_from_system();
        metadata_store[download_key] = entry_metadata;
        data_store["metadata"] = metadata_store;
    } else if (!verified) {
        // Remove metadata when unverifying
        Dictionary metadata_store = data_store.get("metadata", Dictionary());
        if (metadata_store.has(download_key)) {
            metadata_store.erase(download_key);
            data_store["metadata"] = metadata_store;
        }
    }
    
    _save_data();
}

Dictionary ItchDataStore::get_verification_data(const String &download_key) {
    _load_data();
    
    Dictionary metadata_store = data_store.get("metadata", Dictionary());
    return metadata_store.get(download_key, Dictionary());
}

void ItchDataStore::clear_verification(const String &download_key) {
    _load_data();
    
    // Remove from verifications
    Dictionary verifications = data_store.get("verifications", Dictionary());
    if (verifications.has(download_key)) {
        verifications.erase(download_key);
        data_store["verifications"] = verifications;
    }
    
    // Remove from metadata
    Dictionary metadata_store = data_store.get("metadata", Dictionary());
    if (metadata_store.has(download_key)) {
        metadata_store.erase(download_key);
        data_store["metadata"] = metadata_store;
    }
    
    _save_data();
}

void ItchDataStore::clear_all_data() {
    _load_data();
    data_store.clear();
    _save_data();
}
