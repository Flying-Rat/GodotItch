## Itch.io API integration for Godot projects
##
## This class provides functionality to fetch public project metadata from itch.io
## using Open Graph metadata extraction from HTML pages.
##
## Signals:
## - project_fetched(metadata: Dictionary): Emitted when project metadata is successfully retrieved
## - fetch_failed(error_message: String): Emitted when an error occurs during fetching
##
## @tutorial: https://itch.io/docs/api/overview
extends Node

## Emitted when project metadata is successfully fetched from itch.io
## @param metadata: Dictionary containing project information (title, description, image, etc.)
signal project_fetched(metadata: Dictionary)

## Emitted when fetching fails due to network errors, parsing issues, or invalid URLs
## @param error_message: String describing the error that occurred
signal fetch_failed(error_message: String)

## HTTP request handler for making web requests
@onready var _http_request: HTTPRequest = HTTPRequest.new()

## Flag to enable/disable debug logging
var debug_logging: bool = false


func _ready() -> void:
	add_child(_http_request)
	_http_request.request_completed.connect(_on_http_request_completed)


## Fetches public project metadata from itch.io using Open Graph parsing
##
## Supports multiple input formats:
## - Numeric ID: "12345" -> converts to itch.io game URL
## - Full URL: "https://username.itch.io/game-name"
## - Domain-relative: "username.itch.io/game-name"
## - Path-relative: "/username/game-name"
##
## @param project_identifier: String identifying the itch.io project
func fetch_public_project(project_identifier: String) -> void:
	print("[ItchAPI] Starting fetch for: %s" % project_identifier)
	var canonical_url := _canonicalize_url(project_identifier)
	print("[ItchAPI] Requesting URL: %s" % canonical_url)
	var request_error := _http_request.request(canonical_url)
	
	if request_error != OK:
		print("[ItchAPI] ERROR: Request failed with code %d" % request_error)
		fetch_failed.emit("request_start_failed")

## Handles HTTP request completion and parses Open Graph metadata
##
## @param result: HTTPRequest result code
## @param response_code: HTTP status code from server
## @param headers: HTTP response headers
## @param body: Response body as bytes
func _on_http_request_completed(
	result: int,
	response_code: int,
	headers: PackedStringArray,
	body: PackedByteArray
) -> void:
	print("[ItchAPI] Request completed - Result: %d, Response code: %d" % [result, response_code])
	
	if result != HTTPRequest.RESULT_SUCCESS or response_code != 200:
		print("[ItchAPI] ERROR: Request failed - Result: %d, Response: %d" % [result, response_code])
		fetch_failed.emit("http_request_failed")
		return
	
	if body == null or body.size() == 0:
		print("[ItchAPI] ERROR: Empty response body")
		fetch_failed.emit("empty_response")
		return
	
	print("[ItchAPI] Response body size: %d bytes" % body.size())
	var html_content := body.get_string_from_utf8()
	var metadata := _parse_open_graph_metadata(html_content)
	
	if metadata.size() > 0:
		print("[ItchAPI] SUCCESS: Found %d metadata fields" % metadata.size())
		project_fetched.emit(metadata)
	else:
		print("[ItchAPI] ERROR: No metadata extracted")
		fetch_failed.emit("no_metadata_found")

## Parses Open Graph metadata from HTML content
##
## Searches for Open Graph meta tags in content-first format: content="value" property="og:key"
## Falls back to extracting title from <title> tag if no og:title is found
##
## @param html_content: The HTML content to parse
## @return: Dictionary containing Open Graph metadata
func _parse_open_graph_metadata(html_content: String) -> Dictionary:
	print("[ItchAPI] Parsing Open Graph metadata from HTML (%d chars)" % html_content.length())
	var metadata := {}
	
	# Pattern: content first, then property (itch.io format)
	var og_regex := RegEx.new()
	var compile_error := og_regex.compile(
		'<meta[^>]*content="([^"]*)"[^>]*property="og:([^"]+)"[^>]*>'
	)
	
	if compile_error != OK:
		print("[ItchAPI] ERROR: Failed to compile Open Graph regex")
		return metadata
	
	var matches := og_regex.search_all(html_content)
	print("[ItchAPI] Found %d Open Graph matches" % matches.size())
	
	for match_result in matches:
		var content_value := match_result.get_string(1)
		var property_name := match_result.get_string(2)
		print("[ItchAPI] OG property: %s = %s" % [property_name, content_value])
		metadata[property_name] = content_value
	
	# Fallback: Extract title from <title> tag if no og:title found
	if not metadata.has("title"):
		print("[ItchAPI] No og:title found, extracting from <title> tag")
		var title_regex := RegEx.new()
		compile_error = title_regex.compile('<title[^>]*>([^<]*)</title>')
		
		if compile_error == OK:
			var title_match := title_regex.search(html_content)
			if title_match:
				var title_content := title_match.get_string(1).strip_edges()
				print("[ItchAPI] Extracted title: %s" % title_content)
				metadata["title"] = title_content
			else:
				print("[ItchAPI] No <title> tag found")
		else:
			print("[ItchAPI] ERROR: Failed to compile title regex")
	
	print("[ItchAPI] Metadata parsing complete: %d fields total" % metadata.size())
	return metadata


## Converts various input formats to canonical itch.io URLs
##
## @param identifier: The project identifier to canonicalize
## @return: Canonical URL string
func _canonicalize_url(identifier: String) -> String:
	if identifier.is_valid_int():
		return "https://itch.io/g/" + identifier
	elif identifier.begins_with("http"):
		return identifier
	elif "itch.io" in identifier:
		if identifier.begins_with("/"):
			return "https://itch.io" + identifier
		else:
			return "https://" + identifier
	else:
		return identifier
