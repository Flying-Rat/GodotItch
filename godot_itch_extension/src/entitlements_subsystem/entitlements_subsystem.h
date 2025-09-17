#ifndef ENTITLEMENTS_H
#define ENTITLEMENTS_H

#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/callable.hpp>
#include "../core_subsystem/subsystem.h"

// Forward declarations to avoid heavy header includes in this header file
namespace godot {
    class Core;
}

// The project's DataCache is declared in the global namespace (see data_cache.h)
class DataCache;

using namespace godot;

/**
 * Entitlements - Handles purchase verification and entitlement caching
 * Uses CRTP for singleton pattern - no need for explicit static method declarations!
 * 
 * This module provides:
 * - Purchase/entitlement verification against itch.io API
 * - Local caching of verification results with TTL
 * - Fallback to cached results when API is unavailable
 * - Integration with DataCache for persistent storage
 * 
 * Requirements:
 * - Game ID must be configured in project settings (godot_itch/game_id)
 * - API key must be available through Auth module
 */
class Entitlements : public Subsystem<Entitlements>
{
    GDCLASS(Entitlements, Subsystem<Entitlements>);

private:
    
    // Core dependencies
    Core* core = nullptr;
    DataCache* data_cache = nullptr;
    HTTPRequest* http_request = nullptr;
    
    // Verification state
    String pending_download_key;
    bool is_verifying = false;
    bool instance_initialized = false;
    
    // Internal methods
    void _setup_http_request();
    void _cleanup_http_request();
    String _build_verification_url(const String& download_key) const;
    bool _is_cache_valid(const Dictionary& cached_data) const;
    void _store_verification_result(const String& download_key, const Dictionary& result);
    Dictionary _get_cached_verification(const String& download_key) const;
    
    // HTTP response handling
    void _on_verification_response(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body);

protected:
    static void _bind_methods();

public:
    // Lifecycle
    Entitlements();
    ~Entitlements();

    // TODO(jakub.hubacek): refactor scene initialization
    // Scene-aware initialization so HTTPRequest can be added to a scene node
    void initialize_with_scene(Node *scene_node);

    // Override virtual lifecycle methods from Subsystem<Entitlements>
    void initialize_instance() override;
    void shutdown_instance() override;

public:
    
    // Core entitlements API
    void verify_entitlement(const String& download_key);
    bool is_entitled(const String& download_key) const;
    Dictionary get_entitlement_record(const String& download_key) const;
    
    // Cache management
    void clear_entitlement_cache();
    void clear_entitlement_cache_for_key(const String& download_key);
    bool has_cached_entitlement(const String& download_key) const;
    
    // Signals (will be bound in _bind_methods)
    // - entitlement_verified(bool success, Dictionary data)
    // - entitlement_error(String error_message)
};

#endif // ENTITLEMENTS_H