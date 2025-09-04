# Proposal: GDScript addon — public itch.io project metadata

Purpose
- Provide a simple, pure-GDScript Godot plugin that fetches public project metadata from itch.io. Fast to iterate and safe (no server keys required).

Scope
- Implement `itch.get_public_project(id_or_url)` using Godot's `HTTPRequest` to call itch.io public endpoints and return a normalized Dictionary.
- Provide signals: `project_fetched(metadata)` and `fetch_failed(error)`.

Design / API surface
- `class_name ItchAPI` (singleton-like plugin)
- Methods:
  - `get_public_project(project_id_or_url: String) -> void` — starts fetch, emits signal on completion.
  - `parse_project_response(body: String) -> Dictionary` — normalize remote JSON/HTML to a clear shape.

Implementation notes
- Use `HTTPRequest` with HTTPS and reasonable timeouts. Follow redirects.
- Accept either numeric project id or full project URL; canonicalize to API URL.
- No API keys or secrets are used; this only accesses public data.

Edge cases
- Rate limits, network failures, HTML-only pages (fall back to scraping minimal fields), invalid IDs.

Testing
- Add `examples/fetch_demo.tscn` that calls `ItchAPI.get_public_project()` and displays results in a `RichTextLabel`.

Files to add
- `addons/godot_itch/plugin.cfg` — plugin metadata
- `addons/godot_itch/itch.gd` — main API implementation
- `examples/fetch_demo.tscn` + script — demo UI

Next steps
- I can scaffold the `addons/godot_itch` plugin and the demo scene now. Confirm and I'll implement the code and run a quick smoke test.

References
- itch.io integration docs: https://itch.io/docs/itch/integrating/api/

Findings — how to obtain public project metadata

- Preferred: look for an oEmbed/JSON endpoint advertised on the project page. Open the project in a browser and view page source for a link like:
  - `<link rel="alternate" type="application/json+oembed" href="...">` — fetch that href (JSON) for structured metadata.
- Fallback A: parse Open Graph / Twitter meta tags in the page head (`og:title`, `og:description`, `og:image`, etc.). These provide title, description and thumbnail.
- Fallback B: look for JSON-LD (`<script type="application/ld+json">`) which may contain structured fields.
- If none exist, fetch the page HTML and extract minimal metadata by parsing the head section; this is brittle but works for public, read-only data.

Practical steps for the addon

1. Accept `id_or_url` input. If it's numeric, try to resolve to the canonical project URL (`https://itch.io/<user>/<project>`); otherwise assume it's a URL.
2. Fetch the project URL and inspect the response headers and body for an oEmbed link or JSON endpoint. If found, request it and parse JSON.
3. If no JSON endpoint is available, parse the HTML head for Open Graph or JSON-LD and build a normalized Dictionary with keys: `title`, `description`, `author`, `thumbnail`, `url`.
4. Emit `project_fetched(metadata)` on success; emit `fetch_failed(error)` on failure.

Notes on verification (proof of purchase)

- Verifying ownership or getting a "proof of purchase" is a privileged operation in itch.io. Methods:
  - When your game is launched by the itch app, a manifest can inject an API key into the environment. The game can read that environment variable (via `OS.get_environment`) to make server-side API calls directly while running under the app.
  - For general players or public builds, you must use the server-side API from a trusted backend that holds your itch.io API key. The game should contact your backend to request verification — the backend calls itch.io and returns an attestable result.

- Conclusion: you do NOT need a GDExtension just to perform verification; GDScript can read env vars or call your backend. Use a GDExtension only if you need native features beyond HTTP/OS calls.

Sources
- https://itch.io/docs/itch/integrating/api/
- https://itch.io/docs/api/overview
- https://itch.io/docs/itch/integrating/manifest.html
- https://itch.io/docs/creators/widget

