# Detecting the itch.io launcher in HTML5 builds (Itch global)

This document explains how to detect when your HTML5/web build is launched from the itch.io desktop app (the itch launcher) using the documented `Itch` global the app injects into HTML5 pages. It covers the manifest settings that enable the runtime signals, safe usage patterns, example code, and testing tips.

## Quick summary

- The itch desktop app injects a global JavaScript object named `Itch` into the runtime of HTML5 builds that are launched from the app.
- `Itch.args` contains the `args` you put in your `.itch.toml` `[[actions]]` entry.
- If you set `scope` in your manifest action (for example `scope = "profile:me"`), the app will also expose `Itch.env.ITCHIO_API_KEY` so your game can access a session-scoped API key.
- This is the recommended, supported way to detect "launched from itch app" for HTML5 builds.

## Contract (what we get / what to do)

- Inputs:
  - A published build with a `.itch.toml` manifest attached (or a local build validated with `butler`).
  - Optionally: `args` in the manifest action for non-secret signals, or `scope` for a per-session `ITCHIO_API_KEY`.
- Outputs:
  - In-game detection: `Itch` global present -> launched by itch app.
  - Access to `Itch.args` for flags and `Itch.env.ITCHIO_API_KEY` for authentication (if `scope` is requested).
- Error modes / edge cases:
  - `Itch` will be undefined when the build is run in a regular browser (not launched from the itch app) or if the manifest is missing/invalid.
  - `Itch.env.ITCHIO_API_KEY` is only present when the manifest requested `scope` and the user consented; treat it as optional.

## Manifest (example)

Add a `.itch.toml` file to your game folder (top-level of the build directory). Example action that provides both a simple arg and an API scope:

```toml
[[actions]]
name = "play"
path = "index.html"   # or the native entry if a native build
args = ["--launched-via-itch"]
scope = "profile:me"  # request a short-lived session API key (only add if you need identity/auth)
```

Notes:
- `args` are not secret. Use them for flags and detection only.
- `scope` grants `ITCHIO_API_KEY` in `Itch.env` for HTML5 builds and `process.env`/`getenv` for native builds. Treat it as a secret.

## Detecting the Itch global (JavaScript)

A minimal safe detection snippet for an HTML5 runtime:

```js
if (typeof Itch !== 'undefined') {
  // We're running inside the itch desktop app
  const args = Array.isArray(Itch.args) ? Itch.args : [];
  const apiKey = Itch.env && Itch.env.ITCHIO_API_KEY; // may be undefined if no scope requested
  console.log('Launched from itch app, args =', args);
  if (apiKey) {
    // Use apiKey in Authorization header when calling itch APIs server-side or to validate identity
  }
} else {
  // Not launched from the itch app (regular browser)
}
```

### Calling itch API (example) — do this server-side when possible

If you do use `ITCHIO_API_KEY` in the client, prefer to send it to your own backend over HTTPS and have that server call itch's API instead of making sensitive server-side logic depend directly on untrusted client-side checks.

Example fetch to your own server (client -> your server -> itch):

```js
// client
const apiKey = Itch.env && Itch.env.ITCHIO_API_KEY;
if (apiKey) {
  fetch('https://your-server.example.com/validate-itch-token', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ token: apiKey })
  }).then(r => r.json()).then(data => {
    console.log('Server validation result', data);
  });
}
```

Server-side (Node.js example using itchio API):

```js
// server receives token and forwards to itch API
const fetch = require('node-fetch');

async function validateToken(token) {
  const res = await fetch('https://itch.io/api/1/jwt/me', {
    headers: { 'Authorization': token }
  });
  if (!res.ok) throw new Error('validation failed');
  return res.json();
}
```

Treat `token` as sensitive. Do not log it, and only use it over HTTPS.

## Testing locally (simulate the Itch global)

While developing or testing in a regular browser, you can simulate the `Itch` global by adding a tiny test script to your page before loading the game:

```html
<script>
  // simulate itch app injection for local testing
  window.Itch = {
    args: ["--launched-via-itch"],
    env: { ITCHIO_API_KEY: "local-test-token" }
  };
</script>
```

This is useful to exercise code paths that only run when `Itch` is present.

## Godot HTML5 export notes (brief)

If you're using Godot and exporting to HTML5, there are a few ways to pass the data from the page wrapper to your Godot runtime:

- Option A (recommended for simplicity): Add a small script in the exported `index.html` that copies `Itch` into a simple global (for example `window._ITCH`) before the game loads. Then from Godot you can call JavaScript to read `window._ITCH` and pass the values into your game logic.

  Example (index.html):
  ```html
  <script>
    if (typeof Itch !== 'undefined') {
      window._ITCH = Itch; // keep it lightweight
    }
  </script>
  ```

  Example (Godot 4, GDScript using JavaScript singleton):
  ```gdscript
  if Engine.has_singleton("JavaScript"):
      var js = Engine.get_singleton("JavaScript")
      var result = js.eval("window._ITCH ? JSON.stringify(window._ITCH) : null")
      if result != null:
          var itch_obj = JSON.parse(result)
          # use itch_obj.args or itch_obj.env.ITCHIO_API_KEY
  ```

  If you are using Godot 3.x you can still use the `JavaScript.eval` binding if available in your export template, or add a custom HTML wrapper that writes the values into `localStorage` or `Module` before the runtime initializes.

- Option B: Put detection code in a small inline JS layer that communicates with the game via `postMessage` or by setting well-known global variables (same idea as above). Keep communications minimal and secure.

Note: Godot + HTML5 interop details depend on your export template and engine version. Use the simplest approach that works for your pipeline (copying into `window` is straightforward and portable).

## Security & privacy best practices

- Do NOT include secrets in `args` (manifest args are visible to the user and the OS process list).
- Treat `ITCHIO_API_KEY` as a short-lived secret. Do not print it to logs or upload it to public error trackers.
- Prefer server-side verification: send the token to your server and have the server validate it with itch's API instead of trusting client-side-only checks for critical access control.

## Testing with butler and in the itch app

- Validate your manifest locally with `butler validate` to preview action behavior.
- Upload a private build and test launching it from the itch app to confirm `Itch` is injected and `Itch.args`/`Itch.env` are present as expected.

## Troubleshooting

- If `Itch` is undefined when launched from the app, verify that:
  - Your build has a valid `.itch.toml` manifest included in the top-level of the build directory.
  - The action used to launch the build is the one that contains the `args`/`scope`.
  - You tested the exact build that was uploaded (use `butler validate` to simulate locally).

## Checklist before shipping

- [ ] Add `.itch.toml` with intended `[[actions]]` (args and optional scope)
- [ ] Implement runtime detection (see snippets above)
- [ ] If using `ITCHIO_API_KEY`, implement server-side validation and do not leak the token
- [ ] Test in the itch app (private build) and with `butler validate`

---

If you'd like, I can add a small example `index.html` wrapper or a Godot GDScript file to the repo that demonstrates detecting `Itch` and safely sending the token to a test server. Which example would you prefer me to add next?
