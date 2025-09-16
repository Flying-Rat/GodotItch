#ifndef ENTITLEMENTS_H
#define ENTITLEMENTS_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/callable.hpp>
#include "../core/core.h"
#include <godot_cpp/classes/node.hpp>

using namespace godot;

// Forward declarations
class ItchDataCache;

/**
 * Entitlements - Handles purchase verification and entitlement caching
 * 
 * This module provides:
 * - Purchase/entitlement verification against itch.io API
 * - Local caching of verification results with TTL
 * - Fallback to cached results when API is unavailable
 * - Integration with ItchDataCache for persistent storage
 * 
 * Requirements:
 * - Game ID must be configured in project settings (godot_itch/game_id)
 * - API key must be available through ItchAuth module
 */
class Entitlements : public Object {
    GDCLASS(Entitlements, Object);

private:
    
    // Core dependencies
    Core* core = nullptr;
    ItchDataCache* data_cache = nullptr;
    HTTPRequest* http_request = nullptr;
    
    // Verification state
    String pending_download_key;
    bool is_verifying = false;
    bool instance_initialized = false;
    
    // Cache configuration
    // TTL removed: cache entries are persistent. Cache-clearing requests from callers
    // are intentionally ignored by this module (we never remove entries from the
    // underlying ItchDataCache here).
    
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
    // Singleton access
    static Entitlements* get_singleton();
    
    // Lifecycle
    Entitlements();
    ~Entitlements();
    
    // Lifecycle (static wrappers - managed by SubsystemTemplate)
    static void initialize();
    static void shutdown();

    // Instance-level init/shutdown (internal)
    void instance_initialize();
    void instance_shutdown();
    // Scene-aware initialization so HTTPRequest can be added to a scene node
    void initialize_with_scene(Node *scene_node);
    
    // Core entitlements API
    void verify_entitlement(const String& download_key);
    bool is_entitled(const String& download_key) const;
    Dictionary get_entitlement_record(const String& download_key) const;
    
    // Debug utilities
    void dump_debug_state() const;
    
    // Cache management
    void clear_entitlement_cache();
    void clear_entitlement_cache_for_key(const String& download_key);
    bool has_cached_entitlement(const String& download_key) const;
    
    // Configuration
    // (TTL configuration removed — cache entries are kept until cleared)
    
    // Signals (will be bound in _bind_methods)
    // - entitlement_verified(bool success, Dictionary data)
    // - entitlement_error(String error_message)
};

#endif // ENTITLEMENTS_H