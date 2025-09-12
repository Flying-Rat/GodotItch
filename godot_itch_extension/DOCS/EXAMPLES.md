# GodotItch — Examples

This document contains runnable-style examples and explanation of verification flows, cache behavior, and recommended server-authoritative approach.

Persistent JSON example

```
{
  "verifications": {
    "download_key:ABC-123": {
      "verified": true,
      "timestamp": 1690000000,
      "source": "server_verified",
      "verification_meta": {
        "server_id": "api.myserver.com",
        "signature": "...base64..."
      }
    }
  },
  "tokens": {
    "epic": { "access_token": "...", "expires_at": 1690003600 }
  },
  "metadata": { }
}
```

GDScript example — client-only quick verification

```
func check_license(download_key: String) -> void:
    Itch.entitlements.verify_purchase(download_key, Callable(self, "_on_verified"), Callable(self, "_on_error"))

func _on_verified(result: Dictionary) -> void:
    if result.get("verified", false):
        print("Verified — allow play")
    else:
        print("Not verified")

func _on_error(err: Dictionary) -> void:
    print("Verification failed:", err)
```

Pseudo-C++ — server-authoritative verification flow

```cpp
void Entitlements::verify_entitlement(String platform, String id, Callable on_success, Callable on_error) {
    if (data_store->is_verified(platform + ":" + id)) {
        on_success.call(data_store->get_verification_record(platform + ":" + id));
        return;
    }

    Core::get_singleton()->request("/verify/entitlement", {"platform": platform, "id": id},
        Callable(this, "_on_verify_success"), Callable(this, "_on_verify_error"));
}
```

Server response example

```
{
  "verified": true,
  "expires_at": 1690100000,
  "meta": { "platform": "itch", "product_id": "123" },
  "signature": "..."
}
```

Cache TTL

- Default TTL: 24 hours. If cached and not expired, return cached result immediately.
- Offline policy: configurable grace period or deny access depending on product needs.

Error handling

- Errors should use the shared shape: { "code": int, "message": String, "details": Dictionary }

Next

- Add demo scripts under `demo/` that exercise these examples and a unit test for `ItchDataStore` saving/loading.
