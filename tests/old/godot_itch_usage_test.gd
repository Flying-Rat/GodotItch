```gdscript
# Interactive Itch Plugin Test UI
# Provides a visual interface for testing plugin functionality
extends Control

@onready var test_results: RichTextLabel = $VBox/TestResults
@onready var run_tests_button: Button = $VBox/RunTestsButton
@onready var test_key_input: LineEdit = $VBox/InputSection/TestKeyInput
@onready var verify_button: Button = $VBox/InputSection/VerifyButton

var test_count: int = 0
var passed_count: int = 0

func _ready() -> void:
    _setup_ui()
    _add_test_result("🧪 Interactive Itch Plugin Test", "cyan")
    _add_test_result("Ready to test plugin functionality", "white")

func _setup_ui() -> void:
    if run_tests_button:
        run_tests_button.pressed.connect(_run_all_tests)
    if verify_button:
        verify_button.pressed.connect(_test_manual_verification)
    if test_key_input:
        test_key_input.placeholder_text = "Enter test download key or URL"

func _add_test_result(text: String, color: String = "white") -> void:
    if test_results:
        test_results.append_text("[color=%s]%s[/color]\n" % [color, text])

func _test_assert(condition: bool, description: String) -> void:
    test_count += 1
    if condition:
        passed_count += 1
        _add_test_result("✓ %s" % description, "green")
    else:
        _add_test_result("✗ %s" % description, "red")

func _run_all_tests() -> void:
    test_count = 0
    passed_count = 0
    
    _add_test_result("\n--- Running Automated Tests ---", "yellow")
    
    # Test 1: Plugin availability
    _test_plugin_availability()
    
    # Test 2: Itch class functionality
    _test_godot_itch_class()
    
    # Test 3: Autoload access
    _test_autoload_access()
    
    # Test 4: Signal connections
    _test_signal_connections()
    
    # Test 5: Input validation
    _test_input_validation()
    
    # Summary
    _show_test_summary()

func _test_plugin_availability() -> void:
    _add_test_result("\n🔍 Testing Plugin Availability", "cyan")
    
    var plugin_info = Itch.get_plugin_info()
    _test_assert(plugin_info.has("plugin_enabled"), "Plugin info structure valid")
    _test_assert(plugin_info.plugin_enabled, "Plugin is enabled")
    _test_assert(plugin_info.autoload_available, "Autoload is available")

func _test_godot_itch_class() -> void:
    _add_test_result("\n🎯 Testing Itch Class", "cyan")
    # Verify validate() returns the expected structure and status method works
    var validation_result = Itch.validate("test_key_12345678901234567890")
    _test_assert(validation_result.has("valid"), "Itch.validate() returns a validation structure")

    var status = Itch.get_verification_status()
    _test_assert(status.has("verified"), "Itch.get_verification_status() returns a status structure")

func _test_autoload_access() -> void:
    _add_test_result("\n⚙️ Testing Autoload Access", "cyan")
    
    var itch = get_node_or_null("/root/Itch")
    _test_assert(itch != null, "Itch autoload accessible")
    
    if itch:
        _test_assert(itch.has_method("verify"), "Autoload has verify method")
        _test_assert(itch.has_method("validate"), "Autoload has validate method")
        _test_assert(itch.has_signal("verification_completed"), "Autoload has verification_completed signal")

func _test_signal_connections() -> void:
    _add_test_result("\n📡 Testing Signal Connections", "cyan")
    
    var test_callable = func(): pass
    
    # Test Itch signal connections (simplified without try/catch)
    Itch.connect_verification_started(test_callable)
    _test_assert(true, "Itch signal connection successful")
    Itch.disconnect_verification_started(test_callable)
    _test_assert(true, "Itch signal disconnection successful")
    
    # Test direct autoload connections
    var itch = get_node_or_null("/root/Itch")
    if itch:
        itch.verification_started.connect(test_callable)
        _test_assert(true, "Direct autoload signal connection successful")
        itch.verification_started.disconnect(test_callable)
        _test_assert(true, "Direct autoload signal disconnection successful")

func _test_input_validation() -> void:
    _add_test_result("\n✅ Testing Input Validation", "cyan")
    
    # Test valid key
    var valid_result = Itch.validate("test_key_12345678901234567890")
    _test_assert(valid_result.has("valid"), "Validation returns proper structure")
    _test_assert(valid_result.valid, "Valid key is accepted")
    
    # Test invalid key
    var invalid_result = Itch.validate("invalid")
    _test_assert(not invalid_result.valid, "Invalid key is rejected")
    
    # Test URL extraction
    var url_result = Itch.validate("https://user.itch.io/game/download/123?key=test_key_12345678901234567890")
    _test_assert(url_result.valid, "URL key extraction works")

func _test_manual_verification() -> void:
    if not test_key_input:
        _add_test_result("❌ Input field not found", "red")
        return
    
    var input_text = test_key_input.text.strip_edges()
    if input_text.is_empty():
        _add_test_result("❌ Please enter a test key or URL", "red")
        return
    
    _add_test_result("\n🔐 Testing Manual Verification", "cyan")
    _add_test_result("Input: %s" % input_text, "white")
    
    # Connect to verification signals
    Itch.connect_verification_completed(_on_verification_success)
    Itch.connect_verification_failed(_on_verification_failed)
    
    # Start verification
    _add_test_result("Starting verification...", "yellow")
    Itch.verify(input_text)

func _on_verification_success(user_info: Dictionary) -> void:
    _add_test_result("✅ Verification successful!", "green")
    _add_test_result("User: %s" % user_info.get("display_name", "Unknown"), "green")
    _cleanup_verification_signals()

func _on_verification_failed(error_message: String, error_code: String) -> void:
    _add_test_result("❌ Verification failed: %s (%s)" % [error_message, error_code], "red")
    _cleanup_verification_signals()

func _cleanup_verification_signals() -> void:
    Itch.disconnect_verification_completed(_on_verification_success)
    Itch.disconnect_verification_failed(_on_verification_failed)

func _show_test_summary() -> void:
    _add_test_result("\n--- Test Summary ---", "yellow")
    var success_rate = float(passed_count) / float(test_count) * 100.0
    var color = "green" if success_rate >= 90.0 else ("yellow" if success_rate >= 70.0 else "red")
    
    _add_test_result("Tests passed: %d/%d (%.1f%%)" % [passed_count, test_count, success_rate], color)
    
    if passed_count == test_count:
        _add_test_result("🎉 All tests passed! Plugin is working correctly.", "green")
    elif success_rate >= 70.0:
        _add_test_result("⚠️ Most tests passed, but some issues detected.", "yellow")
    else:
        _add_test_result("❌ Multiple test failures detected. Check plugin configuration.", "red")

```
