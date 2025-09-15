#ifndef GODOT_ITCH_USER_H
#define GODOT_ITCH_USER_H

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>

namespace godot
{

    class User
    {
    public:
        String id;
        String username;
        String display_name;

        User() = default;

        static User from_dictionary(const Dictionary &d)
        {
            User u;
            if (d.has("id"))
                u.id = (String)d["id"]; // may be int in API, but store as String
            if (d.has("username"))
                u.username = (String)d["username"];
            if (d.has("display_name"))
                u.display_name = (String)d["display_name"];
            return u;
        }

        Dictionary to_dictionary() const
        {
            Dictionary d;
            if (!id.is_empty())
                d["id"] = id;
            if (!username.is_empty())
                d["username"] = username;
            if (!display_name.is_empty())
                d["display_name"] = display_name;
            return d;
        }
    };

} // namespace godot

#endif // GODOT_ITCH_USER_H
