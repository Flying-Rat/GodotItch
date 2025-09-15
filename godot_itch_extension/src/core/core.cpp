#include "core.h"
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

static Core* s_core_instance = nullptr;

void Core::_bind_methods() {
    // Initialization
    ClassDB::bind_method(D_METHOD("initialize"), &Core::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &Core::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &Core::is_initialized);
    
    // Launch detection
    ClassDB::bind_method(D_METHOD("is_launched_via_itch"), &Core::is_launched_via_itch);
    ClassDB::bind_method(D_METHOD("has_api_key_present"), &Core::has_api_key_present);
    ClassDB::bind_method(D_METHOD("get_launch_api_key"), &Core::get_launch_api_key);
    
    // Configuration
    ClassDB::bind_method(D_METHOD("set_api_key", "api_key"), &Core::set_api_key);
    ClassDB::bind_method(D_METHOD("get_api_key"), &Core::get_api_key);
    ClassDB::bind_method(D_METHOD("set_game_id", "game_id"), &Core::set_game_id);
    ClassDB::bind_method(D_METHOD("get_game_id"), &Core::get_game_id);
    
    // OAuth configuration
    ClassDB::bind_method(D_METHOD("set_oauth_client_id", "client_id"), &Core::set_oauth_client_id);
    ClassDB::bind_method(D_METHOD("set_oauth_redirect_uri", "redirect_uri"), &Core::set_oauth_redirect_uri);
    ClassDB::bind_method(D_METHOD("set_oauth_scope", "scope"), &Core::set_oauth_scope);
    ClassDB::bind_method(D_METHOD("get_oauth_client_id"), &Core::get_oauth_client_id);
    ClassDB::bind_method(D_METHOD("get_oauth_redirect_uri"), &Core::get_oauth_redirect_uri);
    ClassDB::bind_method(D_METHOD("get_oauth_scope"), &Core::get_oauth_scope);
}

Core::Core() {
    s_core_instance = this;
    persistent_cache = nullptr;
}

Core::~Core() {
    if (s_core_instance == this) {
        s_core_instance = nullptr;
    }
    shutdown();
}

Core* Core::get_singleton() {
    if (!s_core_instance) {
        s_core_instance = memnew(Core);
    }
    return s_core_instance;
}

bool Core::initialize() {
    if (initialized) {
        return true;
    }
    
    UtilityFunctions::print("Core: Initializing...");
    
    // Initialize project settings first
    ensure_project_settings();
    
    // Initialize persistent cache
    persistent_cache = ItchDataCache::get_singleton();
    if (persistent_cache) {
        persistent_cache->initialize();
    }
    
    // Detect launch source (moved from Itch class)
    detect_launch_source();
    
    initialized = true;
    UtilityFunctions::print("Core: Initialization complete");
    return true;
}

void Core::shutdown() {
    if (!initialized) {
        return;
    }
    
    UtilityFunctions::print("Core: Shutting down...");
    
    // Shutdown persistent cache
    if (persistent_cache) {
        persistent_cache->shutdown();
        persistent_cache = nullptr;
    }
    
    initialized = false;
    UtilityFunctions::print("Core: Shutdown complete");
}

void Core::detect_launch_source() {
    launched_via_itch = false;
    has_api_key = false;
    launch_api_key = "";
    
    UtilityFunctions::print("Core: Running launch detection...");
    
    // Check environment variable set by itch when scope = "profile:me"
    OS* os = OS::get_singleton();
    if (os) {
        UtilityFunctions::print("Core: Detecting launch source via environment variable...");
        String env_key = os->get_environment("ITCHIO_API_KEY");
        if (!env_key.is_empty()) {
            UtilityFunctions::print("Core: Found ITCHIO_API_KEY in environment variables.");
            launch_api_key = env_key;
            launched_via_itch = true;
            has_api_key = true;
            UtilityFunctions::print("Core: Launched via itch with API key present.");
        }
    }
}

void Core::ensure_project_settings() {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (!ps) {
        return;
    }
    
    if (!ps->has_setting(SETTING_API_KEY)) {
        ps->set_setting(SETTING_API_KEY, "");
    }
    if (!ps->has_setting(SETTING_GAME_ID)) {
        ps->set_setting(SETTING_GAME_ID, "");
    }
    if (!ps->has_setting(SETTING_OAUTH_CLIENT_ID)) {
        ps->set_setting(SETTING_OAUTH_CLIENT_ID, "");
    }
    if (!ps->has_setting(SETTING_OAUTH_REDIRECT_URI)) {
        ps->set_setting(SETTING_OAUTH_REDIRECT_URI, "");
    }
    if (!ps->has_setting(SETTING_OAUTH_SCOPE)) {
        // The only supported scope is "profile:me"
        ps->set_setting(SETTING_OAUTH_SCOPE, "profile:me");
    }
}

String Core::get_api_key_from_settings() const {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (!ps) {
        return "";
    }
    Variant v = ps->get_setting(SETTING_API_KEY);
    if (v.get_type() == Variant::STRING) {
        return v;
    }
    return "";
}

String Core::get_game_id_from_settings() const {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (!ps) {
        return "";
    }
    Variant v = ps->get_setting(SETTING_GAME_ID);
    if (v.get_type() == Variant::STRING) {
        return v;
    }
    return "";
}

// Launch detection methods
bool Core::is_launched_via_itch() const {
    return launched_via_itch;
}

bool Core::has_api_key_present() const {
    return has_api_key;
}

String Core::get_launch_api_key() const {
    return launch_api_key;
}

// Configuration methods
void Core::set_api_key(const String& api_key) {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (ps) {
        ps->set_setting(SETTING_API_KEY, api_key);
    }
}

String Core::get_api_key() const {
    // Check if we have a launch API key first
    if (!launch_api_key.is_empty()) {
        return launch_api_key;
    }
    // Fall back to project settings
    return get_api_key_from_settings();
}

void Core::set_game_id(const String& game_id) {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (ps) {
        ps->set_setting(SETTING_GAME_ID, game_id);
    }
}

String Core::get_game_id() const {
    return get_game_id_from_settings();
}

// OAuth configuration methods
void Core::set_oauth_client_id(const String& client_id) {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (ps) {
        ps->set_setting(SETTING_OAUTH_CLIENT_ID, client_id);
    }
}

void Core::set_oauth_redirect_uri(const String& redirect_uri) {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (ps) {
        ps->set_setting(SETTING_OAUTH_REDIRECT_URI, redirect_uri);
    }
}

void Core::set_oauth_scope(const String& scope) {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (ps) {
        ps->set_setting(SETTING_OAUTH_SCOPE, scope);
    }
}

String Core::get_oauth_client_id() const {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (!ps) {
        return "";
    }
    Variant v = ps->get_setting(SETTING_OAUTH_CLIENT_ID);
    if (v.get_type() == Variant::STRING) {
        return v;
    }
    return "";
}

String Core::get_oauth_redirect_uri() const {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (!ps) {
        return "";
    }
    Variant v = ps->get_setting(SETTING_OAUTH_REDIRECT_URI);
    if (v.get_type() == Variant::STRING) {
        return v;
    }
    return "";
}

String Core::get_oauth_scope() const {
    ProjectSettings* ps = ProjectSettings::get_singleton();
    if (!ps) {
        return "profile:me";
    }
    Variant v = ps->get_setting(SETTING_OAUTH_SCOPE);
    if (v.get_type() == Variant::STRING) {
        return v;
    }
    return "profile:me";
}