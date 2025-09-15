#ifndef ENTITLEMENTS_H
#define ENTITLEMENTS_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/callable.hpp>
#include "../core/core.h"

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
 */
class Entitlements : public Object {
    GDCLASS(Entitlements, Object);

private:
    static Entitlements* s_singleton;
    
    // Core dependencies
    godot::Core* core = nullptr;
    ItchDataCache* data_cache = nullptr;
    HTTPRequest* http_request = nullptr;
    
    // Verification state
    String pending_download_key;
    bool is_verifying = false;
    
    // Cache configuration
    static const int CACHE_TTL_SECONDS = 3600; // 1 hour cache TTL
    
    // Internal methods
    void _setup_http_request();
    String _build_verification_url(const String& download_key, const String& game_id) const;
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
    
    void initialize();
    void shutdown();
    
    // Core entitlements API
    void verify_entitlement(const String& download_key, const String& game_id = "");
    bool is_entitled(const String& download_key) const;
    Dictionary get_entitlement_record(const String& download_key) const;
    
    // Cache management
    void clear_entitlement_cache();
    void clear_entitlement_cache_for_key(const String& download_key);
    bool has_cached_entitlement(const String& download_key) const;
    
    // Configuration
    void set_cache_ttl(int seconds);
    int get_cache_ttl() const;
    
    // Signals (will be bound in _bind_methods)
    // - entitlement_verified(bool success, Dictionary data)
    // - entitlement_error(String error_message)
};

#endif // ENTITLEMENTS_H