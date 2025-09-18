#include "auth_subsystem.h"

// System includes
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/json.hpp>

// Local includes

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

    // OAuth flow
    ClassDB::bind_method(D_METHOD("build_oauth_authorize_url", "client_id", "redirect_uri", "state"), &Auth::build_oauth_authorize_url);
    ClassDB::bind_method(D_METHOD("start_oauth_authorization", "client_id", "redirect_uri", "state"), &Auth::start_oauth_authorization);

    // OAuth token
    ClassDB::bind_method(D_METHOD("set_oauth_token", "token"), &Auth::set_oauth_token);
    ClassDB::bind_method(D_METHOD("get_oauth_token"), &Auth::get_oauth_token);

    // Unified bearer token helpers
    ClassDB::bind_method(D_METHOD("get_bearer_token"), &Auth::get_bearer_token);
    ClassDB::bind_method(D_METHOD("has_bearer_token"), &Auth::has_bearer_token);

    // Define signals
    ADD_SIGNAL(MethodInfo("auth_result", PropertyInfo(Variant::BOOL, "success"), PropertyInfo(Variant::DICTIONARY, "data")));
    ADD_SIGNAL(MethodInfo("auth_error", PropertyInfo(Variant::STRING, "error_message")));

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
void Auth::initialize_instance()
{
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

    initialized = true;
    UtilityFunctions::print("Auth: Initialization complete");
}

// Override virtual shutdown from Subsystem<Auth>
void Auth::shutdown_instance()
{
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
        if (env_key.is_empty())
        {
            // some environments/tools use underscore form
            env_key = os->get_environment("ITCH_IO_API_KEY");
        }
        env_key = env_key.strip_edges(true, true);
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

// Token management
void Auth::set_oauth_token(const String &token)
{
    oauth_token = token;
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