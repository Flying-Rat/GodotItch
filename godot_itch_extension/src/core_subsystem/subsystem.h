#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/memory.hpp>

using namespace godot;

// CRTP (Curiously Recurring Template Pattern) base class for subsystem singletons.
// Derived is the actual subsystem class that inherits from this template.
// This provides a clean two-phase lifecycle: creation + initialization.
template <typename Derived>
class Subsystem : public Object {
private:
    static Derived* s_instance;

protected:
    // Protected constructor - only derived classes can instantiate
    Subsystem() = default;

public:
    // Static singleton access - returns the specific derived type
    static Derived* get_singleton() {
        return s_instance;
    }

    // Phase 1: Create the singleton instance (static, happens first)
    static void create_singleton() {
        if (!s_instance) {
            s_instance = memnew(Derived);
        }
    }

    // Phase 2: Initialize the singleton (static wrapper that calls virtual method)
    static void initialize() {
        if (s_instance) {
            s_instance->initialize_instance();
        }
    }

    // Shutdown the singleton - cleans up the instance
    static void shutdown() {
        if (s_instance) {
            s_instance->shutdown_instance();
            memdelete(s_instance);
            s_instance = nullptr;
        }
    }

    // Virtual methods that derived classes can override for custom initialization
    virtual void initialize_instance() {
        // Default implementation does nothing
        // Derived classes override this for their specific initialization
    }

    virtual void shutdown_instance() {
        // Default implementation does nothing  
        // Derived classes override this for their specific cleanup
    }

public:
    // Virtual destructor for proper cleanup
    virtual ~Subsystem() = default;
};

// Static member definition - each instantiation gets its own static instance
template <typename Derived>
Derived* Subsystem<Derived>::s_instance = nullptr;

#endif // SUBSYSTEM_H
