#include "entitlements.h"

// System includes
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Local includes
#include "../core/core.h"
#include "../core/persistent/itch_data_cache.h"
#include "../submodule_auth/itch_auth.h"
#include "../core/subsystem_template.h"

using namespace godot;

// Singleton lifecycle is handled by SubsystemTemplate<Entitlements>

void Entitlements::_bind_methods()
{
    // Bind core entitlements methods
    ClassDB::bind_method(D_METHOD("verify_entitlement", "download_key"), &Entitlements::verify_entitlement);
    ClassDB::bind_method(D_METHOD("is_entitled", "download_key"), &Entitlements::is_entitled);
    ClassDB::bind_method(D_METHOD("get_entitlement_record", "download_key"), &Entitlements::get_entitlement_record);

    // Initialization methods
    ClassDB::bind_method(D_METHOD("initialize_with_scene", "scene_node"), &Entitlements::initialize_with_scene);

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
    // Clean up any temporary HTTP request safely
    if (http_request) {
        if (is_verifying) {
            http_request->cancel_request();
            is_verifying = false;
        }
        
        // Disconnect signals before cleanup to prevent callbacks
        if (http_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
            http_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
        }
        
        // Only queue_free if object is still in tree
        if (http_request->is_inside_tree()) {
            http_request->queue_free();
        }
        http_request = nullptr;
    }
    
    // Clear state
    pending_download_key = "";
    instance_initialized = false;
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

    // HTTPRequest will be created later in initialize_with_scene() when we have access to scene tree

    instance_initialized = true;
    UtilityFunctions::print("Entitlements: Initialization complete");
}

void Entitlements::initialize_with_scene(Node *scene_node)
{
    UtilityFunctions::print("Entitlements: initialize_with_scene() called");
    // We now use temporary HTTPRequest instances, so no persistent setup needed
    UtilityFunctions::print("Entitlements: Using temporary HTTPRequest approach - no persistent setup required");
}

void Entitlements::instance_shutdown()
{
    UtilityFunctions::print("Entitlements: Shutting down...");

    // Clean up any temporary HTTP request safely
    if (http_request) {
        if (is_verifying) {
            http_request->cancel_request();
        }
        
        // Disconnect signals before cleanup to prevent callbacks
        if (http_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
            http_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
        }
        
        // Only queue_free if object is still in tree
        if (http_request->is_inside_tree()) {
            http_request->queue_free();
        }
        http_request = nullptr;
    }

    core = nullptr;
    data_cache = nullptr;
    is_verifying = false;
    pending_download_key = "";
    instance_initialized = false;
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
    
    // Use a simpler approach - delegate to main Itch class HTTP system
    // This avoids potential conflicts with multiple HTTPRequest instances
    String url = _build_verification_url(download_key);
    if (url.is_empty()) {
        emit_signal("entitlement_error", "Could not build verification URL");
        return;
    }
    
    PackedStringArray headers;
    headers.push_back("User-Agent: GodotItch-Entitlements/1.0");
    
    pending_download_key = download_key;
    is_verifying = true;
    
    UtilityFunctions::print("Entitlements: Making direct HTTP request to:", url);
    
    // Create a temporary HTTPRequest for this single operation
    HTTPRequest* temp_request = memnew(HTTPRequest);
    if (!temp_request) {
        is_verifying = false;
        pending_download_key = "";
        emit_signal("entitlement_error", "Failed to create temporary HTTP request");
        return;
    }
    
    // Configure the temporary request
    temp_request->set_use_threads(false);
    temp_request->set_timeout(10.0);
    
    // Add to scene tree first (required for HTTPRequest to work)
    SceneTree* scene_tree = Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop());
    Node* scene_root = scene_tree ? scene_tree->get_current_scene() : nullptr;
    if (scene_root) {
        scene_root->add_child(temp_request);
        
        // Connect signal AFTER adding to scene tree
        Error connect_result = temp_request->connect("request_completed", Callable(this, "_on_verification_response"));
        if (connect_result != OK) {
            UtilityFunctions::push_error("Entitlements: Failed to connect HTTP signal");
            is_verifying = false;
            pending_download_key = "";
            temp_request->queue_free();
            emit_signal("entitlement_error", "Failed to connect HTTP signal");
            return;
        }
        
        Error result = temp_request->request(url, headers);
        UtilityFunctions::print("Entitlements: HTTP request call completed with result:", String::num_int64(result));
        
        if (result != OK) {
            is_verifying = false;
            pending_download_key = "";
            // Disconnect before cleanup
            if (temp_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
                temp_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
            }
            temp_request->queue_free();
            emit_signal("entitlement_error", "Failed to start HTTP request: " + String::num_int64(result));
            return;
        }
        
        // Store reference to clean up later
        http_request = temp_request;
        UtilityFunctions::print("Entitlements: Started verification request for key: ", download_key);
    } else {
        is_verifying = false;
        pending_download_key = "";
        temp_request->queue_free();
        emit_signal("entitlement_error", "Cannot access scene tree for HTTP request");
        return;
    }
}

void Entitlements::_on_verification_response(int result, int response_code, const PackedStringArray& headers, const PackedByteArray& body)
{
    UtilityFunctions::print("Entitlements: _on_verification_response called - result:", String::num_int64(result), "response_code:", String::num_int64(response_code));
    
    // Critical safety check - ensure object is still valid
    if (!instance_initialized || !http_request) {
        UtilityFunctions::push_error("Entitlements: Response callback called on invalid instance");
        return;
    }
    
    UtilityFunctions::print("Entitlements: Safety checks passed, processing response...");
    
    // Store http_request reference before clearing member variable
    HTTPRequest* temp_http_request = http_request;
    http_request = nullptr;  // Clear member immediately to prevent reuse
    
    is_verifying = false;
    String current_key = pending_download_key;
    pending_download_key = "";
    
    // Helper lambda to cleanup and emit error
    auto cleanup_and_error = [this, temp_http_request](const String& error_msg) {
        UtilityFunctions::push_error("Entitlements: ", error_msg);
        if (temp_http_request && temp_http_request->is_inside_tree()) {
            // Disconnect signal first to prevent further callbacks
            if (temp_http_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
                temp_http_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
            }
            temp_http_request->queue_free();
        }
    };
    
    if (result != HTTPRequest::RESULT_SUCCESS) {
        cleanup_and_error("HTTP request failed with result: " + String::num_int64(result));
        emit_signal("entitlement_error", "Network request failed");
        return;
    }
    
    if (response_code != 200) {
        cleanup_and_error("HTTP response code: " + String::num_int64(response_code));
        emit_signal("entitlement_error", "Server returned error code: " + String::num_int64(response_code));
        return;
    }
    
    // Parse JSON response
    UtilityFunctions::print("Entitlements: Parsing HTTP response...");
    String response_text = body.get_string_from_utf8();
    UtilityFunctions::print("Entitlements: Response text length:", String::num_int64(response_text.length()));
    
    Variant parsed = JSON::parse_string(response_text);
    if (parsed.get_type() == Variant::NIL) {
        cleanup_and_error("Failed to parse JSON response");
        emit_signal("entitlement_error", "Invalid server response format");
        return;
    }
    
    UtilityFunctions::print("Entitlements: JSON parsed successfully");
    Dictionary response_data;
    if (parsed.get_type() == Variant::DICTIONARY) {
        response_data = (Dictionary)parsed;
    } else {
        // Wrap non-dictionary JSON into a result container for consistency
        response_data["result"] = parsed;
    }
    
    // Store in cache with safety check
    UtilityFunctions::print("Entitlements: Storing verification result in cache...");
    if (core && data_cache) {
        _store_verification_result(current_key, response_data);
        UtilityFunctions::print("Entitlements: Result stored in cache");
    } else {
        UtilityFunctions::push_error("Entitlements: Cannot store result - core or data_cache is null");
    }
    
    // Emit success signal
    UtilityFunctions::print("Entitlements: Emitting success signal...");
    emit_signal("entitlement_verified", true, response_data);
    UtilityFunctions::print("Entitlements: Verification complete for key: ", current_key);
    
    // Clean up temporary HTTPRequest safely
    if (temp_http_request && temp_http_request->is_inside_tree()) {
        UtilityFunctions::print("Entitlements: Cleaning up temporary HTTPRequest");
        // Disconnect signal first to prevent any additional callbacks
        if (temp_http_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
            temp_http_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
        }
        temp_http_request->queue_free();
    }
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