#include "itch_auth.h"
#include "core/persistent/itch_data_cache.h"
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static ItchAuth *s_auth_instance = nullptr;

void ItchAuth::_bind_methods()
{
    // Initialization
    ClassDB::bind_method(D_METHOD("initialize"), &ItchAuth::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &ItchAuth::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &ItchAuth::is_initialized);

    // Launch detection
    ClassDB::bind_method(D_METHOD("is_launched_via_itch"), &ItchAuth::is_launched_via_itch);
    ClassDB::bind_method(D_METHOD("has_api_key_present"), &ItchAuth::has_api_key_present);
    ClassDB::bind_method(D_METHOD("get_launch_api_key"), &ItchAuth::get_launch_api_key);

    // API Key management
    ClassDB::bind_method(D_METHOD("set_api_key", "api_key"), &ItchAuth::set_api_key);
    ClassDB::bind_method(D_METHOD("get_api_key"), &ItchAuth::get_api_key);

    // OAuth configuration
    ClassDB::bind_method(D_METHOD("set_oauth_client_id", "client_id"), &ItchAuth::set_oauth_client_id);
    ClassDB::bind_method(D_METHOD("set_oauth_redirect_uri", "redirect_uri"), &ItchAuth::set_oauth_redirect_uri);
    ClassDB::bind_method(D_METHOD("set_oauth_scope", "scope"), &ItchAuth::set_oauth_scope);
    ClassDB::bind_method(D_METHOD("get_oauth_client_id"), &ItchAuth::get_oauth_client_id);
    ClassDB::bind_method(D_METHOD("get_oauth_redirect_uri"), &ItchAuth::get_oauth_redirect_uri);
    ClassDB::bind_method(D_METHOD("get_oauth_scope"), &ItchAuth::get_oauth_scope);

    // OAuth token
    ClassDB::bind_method(D_METHOD("set_oauth_token", "token"), &ItchAuth::set_oauth_token);
    ClassDB::bind_method(D_METHOD("get_oauth_token"), &ItchAuth::get_oauth_token);

    // OAuth flow management
    ClassDB::bind_method(D_METHOD("build_oauth_authorize_url", "client_id", "redirect_uri", "state"), &ItchAuth::build_oauth_authorize_url, DEFVAL(""), DEFVAL(""), DEFVAL(""));
    ClassDB::bind_method(D_METHOD("start_oauth_authorization", "client_id", "redirect_uri", "state"), &ItchAuth::start_oauth_authorization, DEFVAL(""), DEFVAL(""), DEFVAL(""));
}

ItchAuth::ItchAuth()
{
    s_auth_instance = this;
}

ItchAuth::~ItchAuth()
{
    if (s_auth_instance == this)
    {
        s_auth_instance = nullptr;
    }
    shutdown();
}

ItchAuth *ItchAuth::get_singleton()
{
    if (!s_auth_instance)
    {
        s_auth_instance = memnew(ItchAuth);
    }
    return s_auth_instance;
}

bool ItchAuth::initialize()
{
    if (initialized)
    {
        return true;
    }

    UtilityFunctions::print("ItchAuth: Initializing...");

    // Initialize OAuth settings first
    ensure_oauth_settings();

    // Detect launch source (moved from Core and Itch classes)
    detect_launch_source();

    // Load any previously cached OAuth token
    load_token_from_cache();

    initialized = true;
    UtilityFunctions::print("ItchAuth: Initialization complete");
    return true;
}

void ItchAuth::shutdown()
{
    if (!initialized)
    {
        return;
    }

    UtilityFunctions::print("ItchAuth: Shutting down...");
    initialized = false;
    UtilityFunctions::print("ItchAuth: Shutdown complete");
}

void ItchAuth::detect_launch_source()
{
    launched_via_itch = false;
    has_api_key = false;
    launch_api_key = "";

    UtilityFunctions::print("ItchAuth: Running launch detection...");

    // Check environment variable set by itch when scope = "profile:me"
    OS *os = OS::get_singleton();
    if (os)
    {
        UtilityFunctions::print("ItchAuth: Detecting launch source via environment variable...");
        String env_key = os->get_environment("ITCHIO_API_KEY");
        if (!env_key.is_empty())
        {
            UtilityFunctions::print("ItchAuth: Found ITCHIO_API_KEY in environment variables.");
            launch_api_key = env_key;
            launched_via_itch = true;
            has_api_key = true;
            UtilityFunctions::print("ItchAuth: Launched via itch with API key present.");
        }
    }
}

void ItchAuth::ensure_oauth_settings()
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (!ps)
    {
        return;
    }

    if (!ps->has_setting(SETTING_API_KEY))
    {
        ps->set_setting(SETTING_API_KEY, "");
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

String ItchAuth::get_api_key_from_settings() const
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (!ps)
    {
        return "";
    }
    Variant v = ps->get_setting(SETTING_API_KEY);
    if (v.get_type() == Variant::STRING)
    {
        return v;
    }
    return "";
}

// Launch detection methods
bool ItchAuth::is_launched_via_itch() const
{
    return launched_via_itch;
}

bool ItchAuth::has_api_key_present() const
{
    return has_api_key;
}

String ItchAuth::get_launch_api_key() const
{
    return launch_api_key;
}

// API Key management methods
void ItchAuth::set_api_key(const String &api_key)
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (ps)
    {
        ps->set_setting(SETTING_API_KEY, api_key);
    }
}

String ItchAuth::get_api_key() const
{
    // Check if we have a launch API key first
    if (!launch_api_key.is_empty())
    {
        return launch_api_key;
    }
    // Fall back to project settings
    return get_api_key_from_settings();
}

// OAuth configuration methods
void ItchAuth::set_oauth_client_id(const String &client_id)
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (ps)
    {
        ps->set_setting(SETTING_OAUTH_CLIENT_ID, client_id);
    }
}

void ItchAuth::set_oauth_redirect_uri(const String &redirect_uri)
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (ps)
    {
        ps->set_setting(SETTING_OAUTH_REDIRECT_URI, redirect_uri);
    }
}

void ItchAuth::set_oauth_scope(const String &scope)
{
    ProjectSettings *ps = ProjectSettings::get_singleton();
    if (ps)
    {
        ps->set_setting(SETTING_OAUTH_SCOPE, scope);
    }
}

String ItchAuth::get_oauth_client_id() const
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

String ItchAuth::get_oauth_redirect_uri() const
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

String ItchAuth::get_oauth_scope() const
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
String ItchAuth::build_oauth_authorize_url(const String &client_id, const String &redirect_uri, const String &state) const
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

void ItchAuth::start_oauth_authorization(const String &client_id, const String &redirect_uri, const String &state)
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
void ItchAuth::set_oauth_token(const String &token)
{
    oauth_token = token;
    save_token_to_cache();
}

String ItchAuth::get_oauth_token() const
{
    return oauth_token;
}

void ItchAuth::save_token_to_cache()
{
    ItchDataCache *cache = ItchDataCache::get_singleton();
    if (!cache)
    {
        return;
    }
    Dictionary meta;
    meta["oauth_token"] = oauth_token;
    cache->set_verified("oauth_token", true, meta);
}

void ItchAuth::load_token_from_cache()
{
    ItchDataCache *cache = ItchDataCache::get_singleton();
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