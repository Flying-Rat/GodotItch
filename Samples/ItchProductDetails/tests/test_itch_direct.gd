# Direct Itch Plugin API Test
# Tests direct autoload access and basic functionality
extends Node

func _ready():
	print("=== Direct Itch Plugin API Test ===")
	
	# Test 1: Direct autoload access
	test_direct_autoload_access()
	
	# Test 2: Basic method functionality
	test_basic_methods()
	
	# Test 3: Input validation
	test_input_validation()
	
	# Wait a moment then exit
	await get_tree().create_timer(0.3).timeout
	print("=== Direct API Test Complete ===")
	get_tree().quit()

func test_direct_autoload_access():
	print("\n--- Test 1: Direct Autoload Access ---")
	
	# Test direct global access
	var has_global_itch = false
	if "Itch" in globals():
		has_global_itch = true
		print("✓ Itch available as global")
	else:
		print("! Itch not available as global (checking alternatives)")
	
	# Test via scene tree
	var itch_via_tree = get_node_or_null("/root/Itch")
	if itch_via_tree:
		print("✓ Itch autoload accessible via scene tree")
		print("  Type: ", itch_via_tree.get_class())
		print("  Node name: ", itch_via_tree.name)
	else:
		print("✗ Itch autoload NOT accessible via scene tree")
		return
	
	# Test alternative access methods
	var itch_root = get_tree().root.get_node_or_null("Itch")
	print("✓ Root.get_node access: ", itch_root != null)

func test_basic_methods():
	print("\n--- Test 2: Basic Method Functionality ---")
	
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
	
	# Test get_verification_status
	var status = itch.get_verification_status()
	if status.has("verified"):
		print("✓ get_verification_status() returns proper structure")
		print("  Verified: ", status.verified)
		print("  User info: ", status.has("user_info"))
	else:
		print("✗ get_verification_status() returns invalid structure")

func test_input_validation():
	print("\n--- Test 3: Input Validation ---")
	
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

func globals() -> Dictionary:
	# Helper to check global variables
	var result = {}
	# In Godot 4, we can't easily enumerate all globals, so we'll check specific ones
	return result
