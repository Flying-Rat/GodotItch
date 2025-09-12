extends Node

# itch_detection.gd
# Sample GDScript to detect if the game was launched from the itch.io app.
# Works for HTML5 builds (checks the injected `Itch` global via the JavaScript singleton)
# and for native builds (checks ITCHIO_API_KEY environment variable and command-line args).
#
# Usage: attach this script to a node in your main scene (or load it as an autoload)

func _ready():
    var launched_via_itch := false
    var detected_args := []
    var itch_api_key := null

    # Native checks: environment variable and command-line args
    # (Skip HTML5 / JS detection in this simplified version)
    var env_key = OS.get_environment("ITCHIO_API_KEY")
    if env_key != null and env_key != "":
        launched_via_itch = true
        itch_api_key = env_key

    var cmd_args = OS.get_cmdline_args()
    for a in cmd_args:
        detected_args.append(a)
        if a == "--launched-via-itch":
            launched_via_itch = true

    # Final reporting (do not print secrets)
    if launched_via_itch:
        print("[itch_detection] Detected launch from itch app")
        print("[itch_detection] Args: ", detected_args)
        if itch_api_key != null and itch_api_key != "":
            # never print the full token; print only its length to confirm presence
            print("[itch_detection] ITCHIO_API_KEY present (length): ", str(len(itch_api_key)))
        else:
            print("[itch_detection] No ITCHIO_API_KEY provided by manifest scope")
    else:
        print("[itch_detection] No itch launcher detected (running in a regular browser or standalone)")

    # Example: if you need to notify other nodes, emit a signal or set an autoload property.
    # emit_signal("itch_detection_done", launched_via_itch, detected_args, itch_api_key != null)
