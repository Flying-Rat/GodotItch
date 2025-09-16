#include "entitlements.h"
#include "../core/core.h"
#include "../core/persistent/itch_data_cache.h"
#include "../submodule_auth/itch_auth.h"
#include "../core/subsystem_template.h"
#include <godot_cpp/classes/time.hpp>

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


using namespace godot;

// Singleton lifecycle is handled by SubsystemTemplate<Entitlements>

void Entitlements::_bind_methods()
{
    // Bind core entitlements methods
    ClassDB::bind_method(D_METHOD("verify_entitlement", "download_key"), &Entitlements::verify_entitlement);
    ClassDB::bind_method(D_METHOD("is_entitled", "download_key"), &Entitlements::is_entitled);
    ClassDB::bind_method(D_METHOD("get_entitlement_record", "download_key"), &Entitlements::get_entitlement_record);
    
    // Cache management
    ClassDB::bind_method(D_METHOD("has_cached_entitlement", "download_key"), &Entitlements::has_cached_entitlement);
    
    
    // Internal HTTP response handler
    ClassDB::bind_method(D_METHOD("_on_verification_response", "result", "response_code", "headers", "body"), &Entitlements::_on_verification_response);
    
    // Define signals
    ADD_SIGNAL(MethodInfo("entitlement_verified", PropertyInfo(Variant::BOOL, "success"), PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("entitlement_error", PropertyInfo(Variant::STRING, "error_message")));
}

Entitlements* Entitlements::get_singleton()
{
    return SubsystemTemplate<Entitlements>::get_singleton();
}

// Static lifecycle wrappers used by callers (register_types etc.)
void Entitlements::initialize()
{
    SubsystemTemplate<Entitlements>::initialize();
}

void Entitlements::shutdown()
{
    SubsystemTemplate<Entitlements>::shutdown();
}

Entitlements::Entitlements() 
{ 
    UtilityFunctions::print("Entitlements: Constructor called"); 
}

Entitlements::~Entitlements()
{
    if (http_request)
    {
        http_request->queue_free();
    }
}


void Entitlements::instance_initialize()
{
    UtilityFunctions::print("Entitlements: Initializing...");

    // Get Core dependencies
    core = godot::Core::get_singleton();
    if (!core) {
        UtilityFunctions::push_error("Entitlements: Core module not available");
        return;
    }

    data_cache = core->get_persistent_cache();
    if (!data_cache) {
        UtilityFunctions::push_error("Entitlements: ItchDataCache not available from Core");
        return;
    }

    // Setup HTTP request for verification
    _setup_http_request();

    UtilityFunctions::print("Entitlements: Initialization complete");
}

void Entitlements::instance_shutdown()
{
    UtilityFunctions::print("Entitlements: Shutting down...");

    if (http_request)
    {
        http_request->queue_free();
        http_request = nullptr;
    }

    core = nullptr;
    data_cache = nullptr;
    is_verifying = false;
}

void Entitlements::_setup_http_request()
{
    if (http_request) {
        return; // Already setup
    }
    
    http_request = memnew(HTTPRequest);
    if (!http_request) {
        UtilityFunctions::push_error("Entitlements: Failed to create HTTPRequest");
        return;
    }
    
    // Note: HTTPRequest will be managed by the main Itch class which is in the scene tree
    // For now, we'll rely on the main Itch class's HTTP request or handle this differently
    
    // Connect HTTP response signal
    http_request->connect("request_completed", Callable(this, "_on_verification_response"));
    
    UtilityFunctions::print("Entitlements: HTTPRequest setup complete");
}

String Entitlements::_build_verification_url(const String& download_key) const
{
    if (!core) {
        UtilityFunctions::push_error("Entitlements: Core not available for URL building");
        return "";
    }
    
    String api_key = ItchAuth::get_singleton()->get_api_key();
    if (api_key.is_empty()) {
        UtilityFunctions::push_error("Entitlements: API key not available for verification");
        return "";
    }
    
    // Always get game_id from project settings
    String target_game_id = core->get_game_id();
    if (target_game_id.is_empty()) {
        UtilityFunctions::push_error("Entitlements: Game ID not configured in project settings (godot_itch/game_id)");
        return "";
    }
    
    return "https://itch.io/api/1/" + api_key + "/game/" + target_game_id + "/download_keys?download_key=" + download_key;
}

bool Entitlements::_is_cache_valid(const Dictionary& cached_data) const
{
    // Valid if it has a timestamp. TTL-based expiration removed for simplicity.
    return cached_data.has("timestamp");
}

void Entitlements::_store_verification_result(const String& download_key, const Dictionary& result)
{
    if (!data_cache) {
        return;
    }
    
    Dictionary cached_entry;
    cached_entry["timestamp"] = (int64_t)Time::get_singleton()->get_unix_time_from_system();
    cached_entry["result"] = result;
    cached_entry["download_key"] = download_key;
    
    String cache_key = "entitlement_" + download_key;
    data_cache->set_verified(cache_key, true, cached_entry);
    
    UtilityFunctions::print("Entitlements: Stored verification result for key: ", download_key);
}

Dictionary Entitlements::_get_cached_verification(const String& download_key) const
{
    if (!data_cache) {
        return Dictionary();
    }
    
    String cache_key = "entitlement_" + download_key;
    if (!data_cache->is_verified(cache_key)) {
        return Dictionary();
    }
    
    Dictionary cached_data = data_cache->get_verification_data(cache_key);
    if (_is_cache_valid(cached_data)) {
        return cached_data.get("result", Dictionary());
    }
    
    return Dictionary();
}

void Entitlements::verify_entitlement(const String& download_key)
{
    if (download_key.is_empty()) {
        emit_signal("entitlement_error", "Download key cannot be empty");
        return;
    }
    
    // Check cache first
    Dictionary cached_result = _get_cached_verification(download_key);
    if (!cached_result.is_empty()) {
        UtilityFunctions::print("Entitlements: Using cached verification for key: ", download_key);
        emit_signal("entitlement_verified", true, cached_result);
        return;
    }
    
    // Prevent concurrent verifications
    if (is_verifying) {
        emit_signal("entitlement_error", "Verification already in progress");
        return;
    }
    
    if (!http_request) {
        emit_signal("entitlement_error", "HTTP request system not initialized");
        return;
    }
    
    String url = _build_verification_url(download_key);
    if (url.is_empty()) {
        emit_signal("entitlement_error", "Could not build verification URL");
        return;
    }
    
    PackedStringArray headers;
    headers.push_back("User-Agent: GodotItch-Entitlements/1.0");
    
    pending_download_key = download_key;
    is_verifying = true;
    
    Error result = http_request->request(url, headers);
    if (result != OK) {
        is_verifying = false;
        pending_download_key = "";
        emit_signal("entitlement_error", "Failed to start HTTP request: " + String::num_int64(result));
        return;
    }
    
    UtilityFunctions::print("Entitlements: Started verification request for key: ", download_key);
}

void Entitlements::_on_verification_response(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body)
{
    is_verifying = false;
    String current_key = pending_download_key;
    pending_download_key = "";
    
    if (result != HTTPRequest::RESULT_SUCCESS) {
        UtilityFunctions::push_error("Entitlements: HTTP request failed with result: ", String::num_int64(result));
        emit_signal("entitlement_error", "Network request failed");
        return;
    }
    
    if (response_code != 200) {
        UtilityFunctions::push_error("Entitlements: HTTP response code: ", String::num_int64(response_code));
        emit_signal("entitlement_error", "Server returned error code: " + String::num_int64(response_code));
        return;
    }
    
    // Parse JSON response
    String response_text = body.get_string_from_utf8();
    JSON json;
    Error parse_result = json.parse(response_text);
    
    if (parse_result != OK) {
        UtilityFunctions::push_error("Entitlements: Failed to parse JSON response");
        emit_signal("entitlement_error", "Invalid server response format");
        return;
    }
    
    Dictionary response_data = json.get_data();
    
    // Store in cache
    _store_verification_result(current_key, response_data);
    
    // Emit success signal
    emit_signal("entitlement_verified", true, response_data);
    UtilityFunctions::print("Entitlements: Verification complete for key: ", current_key);
}

bool Entitlements::is_entitled(const String& download_key) const
{
    Dictionary cached_result = _get_cached_verification(download_key);
    if (cached_result.is_empty()) {
        return false;
    }
    
    // Check if the response indicates a valid entitlement
    // This depends on itch.io API response format - may need adjustment
    if (cached_result.has("download_keys") && cached_result["download_keys"].get_type() == Variant::ARRAY) {
        Array download_keys = cached_result["download_keys"];
        return download_keys.size() > 0;
    }
    
    return false;
}

Dictionary Entitlements::get_entitlement_record(const String& download_key) const
{
    return _get_cached_verification(download_key);
}




bool Entitlements::has_cached_entitlement(const String& download_key) const
{
    Dictionary cached_result = _get_cached_verification(download_key);
    return !cached_result.is_empty();
}

// TTL configuration removed — cache entries will remain until cleared explicitly