#ifndef SUBSYSTEM_TEMPLATE_H
#define SUBSYSTEM_TEMPLATE_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/memory.hpp>

using namespace godot;

// Lightweight template to centralize singleton lifecycle for subsystems.
// T is expected to be a Godot Object-derived class with a default ctor.
// TODO(jakub.hubacek): instead using this crazy template, use Subsystem abstract parent
template <typename T>
class SubsystemTemplate {
private:
    static T *s_instance;

public:
    static T *get_singleton() {
        return s_instance;
    }

    static void initialize() {
        if (!s_instance) {
            s_instance = memnew(T);
        }
    }

    static void shutdown() {
        if (s_instance) {
            memdelete(s_instance);
            s_instance = nullptr;
        }
    }
};

template <typename T>
T *SubsystemTemplate<T>::s_instance = nullptr;

#endif // SUBSYSTEM_TEMPLATE_H
