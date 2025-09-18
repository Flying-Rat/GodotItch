#include "entitlements_subsystem.h"

// System includes
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Local includes
#include "../core_subsystem/core_subsystem.h"
#include "../auth_subsystem/auth_subsystem.h"

using namespace godot;

// Explicit template instantiation for Entitlements CRTP
template class Subsystem<Entitlements>;

// Constants (internal linkage)
namespace {
constexpr int HTTP_OK = 200;
constexpr double HTTP_TIMEOUT_SECONDS = 10.0;
constexpr const char* USER_AGENT = "GodotItch-Entitlements/1.0";
} // anonymous namespace

void Entitlements::_bind_methods()
{
    // Bind core entitlements methods
    ClassDB::bind_method(D_METHOD("verify_entitlement", "download_key"), &Entitlements::verify_entitlement);

    // Initialization methods
    ClassDB::bind_method(D_METHOD("initialize_with_scene", "scene_node"), &Entitlements::initialize_with_scene);

    // Internal HTTP response handler
    ClassDB::bind_method(D_METHOD("_on_verification_response", "result", "response_code", "headers", "body"), &Entitlements::_on_verification_response);

    // Define signals
    ADD_SIGNAL(MethodInfo("entitlement_verified", PropertyInfo(Variant::BOOL, "success"), PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("entitlement_error", PropertyInfo(Variant::STRING, "error_message")));
}

Entitlements::Entitlements()
{ 
    // Constructor - keep minimal logging
}

Entitlements::~Entitlements()
{
    _cleanup_http_request();

    // Clear state
    pending_download_key = "";
    instance_initialized = false;
}

void Entitlements::_cleanup_http_request()
{
    if (!http_request) {
        return;
    }

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


// Override virtual initialization from Subsystem<Entitlements>
void Entitlements::initialize_instance() 
{
    // Initialization started

    // Get Core dependencies
    core = godot::Core::get_singleton();
    if (!core)
    {
        UtilityFunctions::push_error("Entitlements: Core module not available");
        return;
    }
    instance_initialized = true;
}

void Entitlements::initialize_with_scene(Node *scene_node)
{
    // No scene-specific setup required for current temporary HTTPRequest approach
}

void Entitlements::shutdown_instance()
{
    // Shutting down

    _cleanup_http_request();

    core = nullptr;
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

    // Always get game_id from project settings
    String target_game_id = core->get_game_id();
    if (target_game_id.is_empty()) {
        UtilityFunctions::push_error("Entitlements: Game ID not configured in project settings (godot_itch/game_id)");
        return "";
    }

    // Determine endpoint based on the type of credential available
    String token;
    if (Auth::get_singleton()) {
        token = Auth::get_singleton()->get_bearer_token();
    }

    // Heuristic: JWT tokens have exactly two dots (header.payload.signature)
    int dot_count = 0;
    for (int i = 0; i < token.length(); i++) {
        if (token[i] == '.') {
            dot_count++;
        }
    }
    const bool looks_like_jwt = (dot_count == 2);

    String auth_kind = looks_like_jwt ? "jwt" : "key";
    return "https://itch.io/api/1/" + auth_kind + "/game/" + target_game_id + "/download_keys?download_key=" + download_key;
}

bool Entitlements::_is_cache_valid(const Dictionary& cached_data) const
{
    return cached_data.has("timestamp");
}

void Entitlements::verify_entitlement(const String& download_key)
{
    // Input validation
    if (download_key.is_empty()) {
        UtilityFunctions::push_error("Entitlements: Download key cannot be empty");
        emit_signal("entitlement_error", "Download key cannot be empty");
        return;
    }

    if (download_key.length() < 10) {
        UtilityFunctions::push_error("Entitlements: Download key appears to be too short");
        emit_signal("entitlement_error", "Download key appears to be invalid (too short)");
        return;
    }

    if (!instance_initialized) {
        UtilityFunctions::push_error("Entitlements: Module not properly initialized");
        emit_signal("entitlement_error", "Entitlements module not initialized");
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

    String token = Auth::get_singleton()->get_bearer_token();
    if (token.is_empty()) {
        UtilityFunctions::push_error("Entitlements: No OAuth/launcher token available for verification");
        emit_signal("entitlement_error", "User not authenticated (missing OAuth/launcher token)");
        return;
    }

    PackedStringArray headers;
    headers.push_back(String("User-Agent: ") + USER_AGENT);
    headers.push_back("Authorization: Bearer " + token);

    pending_download_key = download_key;
    is_verifying = true;

    UtilityFunctions::print("Entitlements: Making direct HTTP request to: ", url);

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
    temp_request->set_timeout(HTTP_TIMEOUT_SECONDS);

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
        // Request started successfully
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
    UtilityFunctions::print("Entitlements: _on_verification_response called - result: ", String::num_int64(result), ", response_code:", String::num_int64(response_code));

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
    
    // Simplified error handling: if the request failed, cleanup and emit error
    if (result != HTTPRequest::RESULT_SUCCESS) {
        String error_description = "HTTP request failed (code " + String::num_int64(result) + ")";
        UtilityFunctions::push_error("Entitlements: ", error_description);
        // Clean up temporary request if it's still valid
        if (temp_http_request && temp_http_request->is_inside_tree()) {
            if (temp_http_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
                temp_http_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
            }
            temp_http_request->queue_free();
        }
        emit_signal("entitlement_error", "Network request failed: " + error_description);
        return;
    }

    if (response_code != HTTP_OK) {
        String error_description = "HTTP response code: " + String::num_int64(response_code);
        UtilityFunctions::push_error("Entitlements: ", error_description);
        if (temp_http_request && temp_http_request->is_inside_tree()) {
            if (temp_http_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
                temp_http_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
            }
            temp_http_request->queue_free();
        }
        emit_signal("entitlement_error", "Server returned error code: " + String::num_int64(response_code));
        return;
    }

    // Parse JSON response
    String response_text = body.get_string_from_utf8();
    UtilityFunctions::print("Entitlements: Response text length:", String::num_int64(response_text.length()));

    Variant parsed = JSON::parse_string(response_text);
    if (parsed.get_type() == Variant::NIL) {
        String error_description = "Failed to parse JSON response";
        UtilityFunctions::push_error("Entitlements: ", error_description);
        if (temp_http_request && temp_http_request->is_inside_tree()) {
            if (temp_http_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
                temp_http_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
            }
            temp_http_request->queue_free();
        }
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

    // Print full response body for debugging (requested)
    UtilityFunctions::print("Entitlements: Full response body:\n" + response_text);

    // Emit success signal
    emit_signal("entitlement_verified", true, response_data);
    UtilityFunctions::print("Entitlements: Verification complete for key: ", current_key);

    // Clean up temporary HTTPRequest safely
    if (temp_http_request && temp_http_request->is_inside_tree()) {
        // Disconnect signal first to prevent any additional callbacks
        if (temp_http_request->is_connected("request_completed", Callable(this, "_on_verification_response"))) {
            temp_http_request->disconnect("request_completed", Callable(this, "_on_verification_response"));
        }
        temp_http_request->queue_free();
    }
}

