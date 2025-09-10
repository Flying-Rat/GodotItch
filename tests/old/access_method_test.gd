# Comprehensive Access Method and Basic Functionality Test
# Tests different ways to access the plugin and validates core functionality
extends Node

func _ready():
    print("=== Itch Access Method & Basic Functionality Test ===")
    
    # Test 1: Check if autoload exists immediately
    test_immediate_autoload_access()
    
    # Test 2: Test Itch class access
    test_godot_itch_class_access()
    
    # Test 3: Test timing with call_deferred
    call_deferred("test_deferred_access")
    
    # Test 4: Signal connections
    call_deferred("test_signal_connections")
    
    # Test 5: Basic method functionality
    call_deferred("test_basic_methods")
    
    # Test 6: Input validation
    call_deferred("test_input_validation")
    
    # Wait a moment then exit
    await get_tree().create_timer(0.7).timeout
    print("=== Access Method & Basic Functionality Test Complete ===")
    get_tree().quit()

func test_immediate_autoload_access():
    print("\n--- Test 1: Immediate Autoload Access ---")
    var itch = get_node_or_null("/root/Itch")
    if itch:
        print("✓ Autoload found immediately")
        print("  Type: ", itch.get_class())
        print("  Node name: ", itch.name)
        print("  Has verify method: ", itch.has_method("verify"))
        print("  Has signals: ", itch.get_signal_list().size(), " signals")
        
        # Test basic functionality
        var status = itch.get_verification_status()
        print("  Status method works: ", status.has("verified"))
        
        var validation = itch.validate("test_key_123456789012345678901")
        print("  Validation method works: ", validation.has("valid"))
        
        # Test alternative access methods
        var itch_root = get_tree().root.get_node_or_null("Itch")
        print("  Root.get_node access: ", itch_root != null)
    else:
        print("✗ Autoload NOT found immediately")

func test_godot_itch_class_access():
    print("\n--- Test 2: Itch Class Access ---")

    # Check if Itch class is available
    if ClassDB.class_exists("Itch"):
        print("✓ Itch class exists in ClassDB")
    else:
        print("! Itch class not in ClassDB (this is normal for custom classes)")

    # Test static methods
    var plugin_info = Itch.get_plugin_info()
    print("✓ Itch.get_plugin_info() works")
    print("  Plugin enabled: ", plugin_info.plugin_enabled)
    print("  Autoload available: ", plugin_info.autoload_available)

    var status = Itch.get_verification_status()
    print("✓ Itch.get_verification_status() works")
    print("  Verified: ", status.verified)

    var validation = Itch.validate("test_key_123456789012345678901")
    print("✓ Itch.validate() works")
    print("  Valid: ", validation.valid)
    print("  Key: ", validation.key)

func test_deferred_access():
    print("\n--- Test 3: Deferred Access ---")
    var itch = get_node_or_null("/root/Itch")
    if itch:
        print("✓ Autoload found in deferred call")
    else:
        print("✗ Autoload NOT found in deferred call")

func test_signal_connections():
    print("\n--- Test 4: Signal Connection Methods ---")
    
    # Method 1: Direct connection
    var itch = get_node_or_null("/root/Itch")
    if itch:
        var callable = Callable(self, "_dummy_signal_handler")
        itch.verification_started.connect(callable)
        print("✓ Direct signal connection works")
        itch.verification_started.disconnect(callable)
    
    # Method 2: Itch class connection
    var callable = Callable(self, "_dummy_signal_handler")
    Itch.connect_verification_started(callable)
    print("✓ Itch signal connection works")
    Itch.disconnect_verification_started(callable)

func test_basic_methods():
    print("\n--- Test 5: Basic Method Functionality ---")
    
    var itch = get_node_or_null("/root/Itch")
    if not itch:
        print("✗ Cannot test methods - autoload not found")
        return
    
    # Test method availability
    var methods_to_test = ["verify", "validate", "get_verification_status", "clear_verification"]
    for method_name in methods_to_test:
        if itch.has_method(method_name):
            print("✓ Method '%s' available" % method_name)
        else:
            print("✗ Method '%s' missing" % method_name)
    
    # Test get_verification_status structure
    var status = itch.get_verification_status()
    if status.has("verified"):
        print("✓ get_verification_status() returns proper structure")
        print("  Verified: ", status.verified)
        print("  User info: ", status.has("user_info"))
    else:
        print("✗ get_verification_status() returns invalid structure")

func test_input_validation():
    print("\n--- Test 6: Input Validation ---")
    
    var itch = get_node_or_null("/root/Itch")
    if not itch:
        print("✗ Cannot test validation - autoload not found")
        return
    
    # Test valid key format
    var valid_key = "test_key_12345678901234567890"
    var valid_result = itch.validate(valid_key)
    if valid_result.has("valid") and valid_result.valid:
        print("✓ Valid key format accepted")
        print("  Key: ", valid_result.key)
    else:
        print("✗ Valid key format rejected: ", valid_result.get("error", "unknown error"))
    
    # Test invalid key formats
    var invalid_keys = ["", "short", "abc", "123"]
    for invalid_key in invalid_keys:
        var invalid_result = itch.validate(invalid_key)
        if invalid_result.has("valid") and not invalid_result.valid:
            print("✓ Invalid key '%s' properly rejected" % invalid_key)
        else:
            print("✗ Invalid key '%s' incorrectly accepted" % invalid_key)
    
    # Test URL extraction
    var test_url = "https://user.itch.io/game/download/123?key=test_key_12345678901234567890"
    var url_result = itch.validate(test_url)
    if url_result.has("valid") and url_result.valid:
        print("✓ URL key extraction works")
        print("  Extracted key: ", url_result.key)
    else:
        print("✗ URL key extraction failed: ", url_result.get("error", "unknown error"))

func _dummy_signal_handler():
    pass