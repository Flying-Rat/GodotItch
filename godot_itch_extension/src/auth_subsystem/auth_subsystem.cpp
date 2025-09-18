#include "auth_subsystem.h"

// System includes
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/json.hpp>

// Local includes
#include "../core_subsystem/persistent/data_cache.h"

using namespace godot;

// Explicit template instantiation for Auth CRTP
template class Subsystem<Auth>;

// Constants
namespace {
    constexpr int HTTP_OK = 200;
    constexpr double HTTP_TIMEOUT_SECONDS = 10.0;
    constexpr const char* USER_AGENT = "GodotItch-Auth/1.0";
}

void Auth::_bind_methods()
{
    // Initialization
    ClassDB::bind_method(D_METHOD("is_initialized"), &Auth::is_initialized);

    // Launch detection
    ClassDB::bind_method(D_METHOD("is_launched_via_itch"), &Auth::is_launched_via_itch);
    ClassDB::bind_method(D_METHOD("has_api_key_present"), &Auth::has_api_key_present);
    ClassDB::bind_method(D_METHOD("get_launch_api_key"), &Auth::get_launch_api_key);

    // OAuth configuration
    ClassDB::bind_method(D_METHOD("set_oauth_client_id", "client_id"), &Auth::set_oauth_client_id);
    ClassDB::bind_method(D_METHOD("set_oauth_redirect_uri", "redirect_uri"), &Auth::set_oauth_redirect_uri);
    ClassDB::bind_method(D_METHOD("set_oauth_scope", "scope"), &Auth::set_oauth_scope);
    ClassDB::bind_method(D_METHOD("get_oauth_client_id"), &Auth::get_oauth_client_id);
    ClassDB::bind_method(D_METHOD("get_oauth_redirect_uri"), &Auth::get_oauth_redirect_uri);
    ClassDB::bind_method(D_METHOD("get_oauth_scope"), &Auth::get_oauth_scope);

    // OAuth token
    ClassDB::bind_method(D_METHOD("set_oauth_token", "token"), &Auth::set_oauth_token);
    ClassDB::bind_method(D_METHOD("get_oauth_token"), &Auth::get_oauth_token);
    ClassDB::bind_method(D_METHOD("build_oauth_authorize_url"), &Auth::build_oauth_authorize_url);
    ClassDB::bind_method(D_METHOD("start_oauth_authorization"), &Auth::start_oauth_authorization);

    // Unified bearer token helpers
    ClassDB::bind_method(D_METHOD("get_bearer_token"), &Auth::get_bearer_token);
    ClassDB::bind_method(D_METHOD("has_bearer_token"), &Auth::has_bearer_token);

    // Define signals
    ADD_SIGNAL(MethodInfo("auth_result", PropertyInfo(Variant::BOOL, "success"), PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("auth_error", PropertyInfo(Variant::STRING, "error_message")));

    // Public API
    ClassDB::bind_method(D_METHOD("get_credentials_info"), &Auth::get_credentials_info);

    // Internal HTTP callback
    ClassDB::bind_method(D_METHOD("_on_auth_result", "result", "response_code", "headers", "body"), &Auth::_on_auth_result);
}

Auth::Auth()
{
    // Constructor implementation
}

Auth::~Auth()
{
    // Destructor implementation
}

// Override virtual initialization from Subsystem<Auth>
void Auth::initialize_instance() {
    if (initialized)
    {
        return;
    }

    UtilityFunctions::print("Auth: Initializing...");

    // Initialize OAuth settings first
    ensure_oauth_settings();

    // Detect launch source (moved from Core and Itch classes)
    // call internal helper
    do_detect_launch_source();

    // Load any previously cached OAuth token
    load_token_from_cache();

    initialized = true;
    UtilityFunctions::print("Auth: Initialization complete");
}

// Override virtual shutdown from Subsystem<Auth>
void Auth::shutdown_instance() {
    if (!initialized)
    {
        return;
    }

    UtilityFunctions::print("Auth: Shutting down...");
    initialized = false;
    UtilityFunctions::print("Auth: Shutdown complete");
}

void Auth::do_detect_launch_source()
{
    launched_via_itch = false;
    has_api_key = false;
    launch_api_key = "";

    UtilityFunctions::print("Auth: Running launch detection...");

    // Check environment variable set by itch when scope = "profile:me"
    OS *os = OS::get_singleton();
    if (os)
    {
    UtilityFunctions::print("Auth: Detecting launch source via environment variable...");
        String env_key = os->get_environment("ITCHIO_API_KEY");
        if (!env_key.is_empty())
        {
            UtilityFunctions::print("Auth: Found ITCHIO_API_KEY in environment variables.");
            launch_api_key = env_key;
            launched_via_itch = true;
            has_api_key = true;
            UtilityFunctions::print("Auth: Launched via itch with API key present.");
        }
    }
}

void Auth::ensure_oauth_settings()
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (!ps)
    {
        return;
    }

    if (!ps->has_setting(SETTING_OAUTH_CLIENT_ID))
    {
        ps->set_setting(SETTING_OAUTH_CLIENT_ID, "");
    }
    if (!ps->has_setting(SETTING_OAUTH_REDIRECT_URI))
    {
        ps->set_setting(SETTING_OAUTH_REDIRECT_URI, "");
    }
    if (!ps->has_setting(SETTING_OAUTH_SCOPE))
    {
        // The only supported scope is "profile:me"
        ps->set_setting(SETTING_OAUTH_SCOPE, "profile:me");
    }
}

void godot::Auth::_on_auth_result(int result, int response_code, const PackedStringArray &headers, const PackedByteArray &body)
{
    UtilityFunctions::push_warning("Auth: _on_auth_result called");
    // Handle the authentication result for credentials/info
    auto cleanup = [&]() {
        if (http_request) {
            if (http_request->is_connected("request_completed", Callable(this, "_on_auth_result"))) {
                http_request->disconnect("request_completed", Callable(this, "_on_auth_result"));
            }
            http_request->queue_free();
            http_request = nullptr;
        }
    };

    if (result != OK) {
        UtilityFunctions::push_error("Auth: HTTP transport error: " + String::num_int64(result));
        emit_signal("auth_error", String("HTTP transport error: ") + String::num_int64(result));
        cleanup();
        return;
    }

    if (response_code != HTTP_OK) {
        UtilityFunctions::push_error("Auth: HTTP response code: " + String::num_int64(response_code));
        emit_signal("auth_error", String("HTTP error ") + String::num_int64(response_code));
        cleanup();
        return;
    }

    String body_string;
    body_string.parse_utf8((const char*)body.ptr(), body.size());
    Variant parsed = JSON::parse_string(body_string);
    Dictionary data;
    if (parsed.get_type() == Variant::DICTIONARY) {
        data = parsed;
    } else {
        emit_signal("auth_error", String("Invalid JSON response"));
        cleanup();
        return;
    }

    bool success = !data.has("errors");
    UtilityFunctions::push_warning("Auth: _on_auth_result success=" + String(success ? "true" : "false"));
    emit_signal("auth_result", success, data);
    cleanup();
}

// Launch detection methods
bool Auth::is_launched_via_itch() const
{
    return launched_via_itch;
}

bool Auth::has_api_key_present() const
{
    return has_api_key;
}

String Auth::get_launch_api_key() const
{
    return launch_api_key;
}

// OAuth configuration methods
void Auth::set_oauth_client_id(const String &client_id)
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (ps)
    {
        ps->set_setting(SETTING_OAUTH_CLIENT_ID, client_id);
    }
}

void Auth::set_oauth_redirect_uri(const String &redirect_uri)
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (ps)
    {
        ps->set_setting(SETTING_OAUTH_REDIRECT_URI, redirect_uri);
    }
}

void Auth::set_oauth_scope(const String &scope)
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (ps)
    {
        ps->set_setting(SETTING_OAUTH_SCOPE, scope);
    }
}

String Auth::get_oauth_client_id() const
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (!ps)
    {
        return "";
    }
    Variant v = ps->get_setting(SETTING_OAUTH_CLIENT_ID);
    if (v.get_type() == Variant::STRING)
    {
        return v;
    }
    return "";
}

String Auth::get_oauth_redirect_uri() const
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (!ps)
    {
        return "";
    }
    Variant v = ps->get_setting(SETTING_OAUTH_REDIRECT_URI);
    if (v.get_type() == Variant::STRING)
    {
        return v;
    }
    return "";
}

String Auth::get_oauth_scope() const
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (!ps)
    {
        return "profile:me";
    }
    Variant v = ps->get_setting(SETTING_OAUTH_SCOPE);
    if (v.get_type() == Variant::STRING)
    {
        return v;
    }
    return "profile:me";
}

// OAuth flow management methods
String Auth::build_oauth_authorize_url(const String &client_id, const String &redirect_uri, const String &state) const
{
    String cid = client_id.is_empty() ? get_oauth_client_id() : client_id;
    String ruri = redirect_uri.is_empty() ? get_oauth_redirect_uri() : redirect_uri;
    if (ruri.is_empty())
    {
        ruri = "urn:ietf:wg:oauth:2.0:oob";
    }
    String scope = get_oauth_scope(); // enforced to "profile:me"

    if (cid.is_empty())
    {
        UtilityFunctions::push_error("OAuth client_id must be set (either via parameters or project settings).");
        return "";
    }

    // Encode parameters
    String cid_enc = cid.uri_encode();
    String scope_enc = scope.uri_encode();
    String url = "https://itch.io/user/oauth?client_id=" + cid_enc + "&scope=" + scope_enc + "&response_type=token";
    if (!ruri.is_empty())
    {
        String ruri_enc = ruri.uri_encode();
        url += "&redirect_uri=" + ruri_enc;
    }
    if (!state.is_empty())
    {
        url += "&state=" + state.uri_encode();
    }
    return url;
}

void Auth::start_oauth_authorization(const String &client_id, const String &redirect_uri, const String &state)
{
    String url = build_oauth_authorize_url(client_id, redirect_uri, state);
    if (url.is_empty())
    {
        return;
    }
    OS *os = OS::get_singleton();
    if (os)
    {
        bool ok = os->shell_open(url) == Error::OK;
        if (!ok)
        {
            UtilityFunctions::push_error("Failed to open OAuth authorization URL in browser.");
        }
    }
}

void godot::Auth::get_credentials_info()
{
    UtilityFunctions::push_warning("Auth: get_credentials_info called");

    // Prefer OAuth/API key if available; otherwise use launcher key via unified accessor
    const String token = get_bearer_token();
    if (token.is_empty()) {
        UtilityFunctions::push_error("Auth: No OAuth token or launcher key available");
        emit_signal("auth_error", "User not authenticated (missing OAuth token and launcher key)");
        return;
    }

    // Detect if token looks like a JWT (three segments separated by '.')
    int dot_count = 0;
    for (int i = 0; i < token.length(); i++) {
        if (token[i] == '.') {
            dot_count++;
        }
    }
    const bool looks_like_jwt = (dot_count == 2);

    String url;
    PackedStringArray headers;
    headers.push_back(String("User-Agent: ") + USER_AGENT);
    headers.push_back(String("Authorization: Bearer ") + token);

    if (looks_like_jwt) {
        url = "https://itch.io/api/1/jwt/credentials/info";
        UtilityFunctions::print("Auth: Using JWT for credentials/info");
    } else {
        // Treat as API key; use header-based auth to avoid leaking key in URL
        url = "https://itch.io/api/1/key/credentials/info";
        UtilityFunctions::print("Auth: Using API key for credentials/info");
    }

    UtilityFunctions::print("Auth: Making direct HTTP request to: ", url);

    // Create a temporary HTTPRequest for this single operation
    HTTPRequest* temp_request = memnew(HTTPRequest);
    if (!temp_request) {
        emit_signal("auth_error", "Failed to create temporary HTTP request");
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
        Error connect_result = temp_request->connect("request_completed", Callable(this, "_on_auth_result"));
        if (connect_result != OK) {
            UtilityFunctions::push_error("Auth: Failed to connect HTTP signal");
            temp_request->queue_free();
            emit_signal("auth_error", "Failed to connect HTTP signal");
            return;
        }

        Error result = temp_request->request(url, headers);
        UtilityFunctions::print("Auth: HTTP request call completed with result:", String::num_int64(result));

        if (result != OK) {
            // Disconnect before cleanup
            if (temp_request->is_connected("request_completed", Callable(this, "_on_auth_result"))) {
                temp_request->disconnect("request_completed", Callable(this, "_on_auth_result"));
            }
            temp_request->queue_free();
            emit_signal("auth_error", "Failed to start HTTP request: " + String::num_int64(result));
            return;
        }

        // Store reference to clean up later
        http_request = temp_request;
        UtilityFunctions::print("Auth: Started credentials/info request");
    } else {
        temp_request->queue_free();
        emit_signal("auth_error", "Cannot access scene tree for HTTP request");
        return;
    }
}

// Token management
void Auth::set_oauth_token(const String &token)
{
    oauth_token = token;
    save_token_to_cache();
}

String Auth::get_oauth_token() const
{
    return oauth_token;
}

String Auth::get_bearer_token() const
{
    // Prefer OAuth token if present; otherwise use launch_api_key (itch launcher provides a short-lived token)
    if (!oauth_token.is_empty()) {
        return oauth_token;
    }
    if (!launch_api_key.is_empty()) {
        return launch_api_key;
    }
    return String("");
}

bool Auth::has_bearer_token() const
{
    return !oauth_token.is_empty() || !launch_api_key.is_empty();
}

void Auth::save_token_to_cache()
{
    DataCache *cache = DataCache::get_singleton();
    if (!cache)
    {
        return;
    }
    Dictionary meta;
    meta["oauth_token"] = oauth_token;
    cache->set_verified("oauth_token", true, meta);
}

void Auth::load_token_from_cache()
{
    DataCache *cache = DataCache::get_singleton();
    if (!cache)
    {
        return;
    }
    Dictionary data = cache->get_verification_data("oauth_token");
    if (data.has("oauth_token"))
    {
        oauth_token = data["oauth_token"];
    }
}