# save as res://gd_test_request.gd
extends Node

func _ready():
	var r = HTTPRequest.new()
	add_child(r)
	r.connect("request_completed", Callable(self, "_on_req"))
	var headers = ["User-Agent: GodotItch/1.0"]
	r.request("http://example.com/", headers)
	print("GDScript: requested example.com")

func _on_req(result, response_code, headers, body):
	print("GDScript got", result, response_code)
