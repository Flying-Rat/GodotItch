#ifndef STUB_HELPERS_H
#define STUB_HELPERS_H

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

// Small helpers to build common stub responses used across subsystems.
static inline Dictionary make_stub_status() {
    Dictionary d;
    d["status"] = "stub_implementation";
    return d;
}

static inline Dictionary make_stub_with(const String &key, const Variant &value) {
    Dictionary d = make_stub_status();
    d[key] = value;
    return d;
}

static inline Array make_stub_array_with(const String &key, const Variant &value) {
    Array a;
    Dictionary d = make_stub_with(key, value);
    a.append(d);
    return a;
}

#endif // STUB_HELPERS_H
