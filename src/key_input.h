#ifndef KEY_INPUT_H

enum {
#define Key(name, str) KEY_##name,
#include "key_list.inc"
#undef Key
    KEY_MAX
};

enum {
    KEY_MODIFIER_CTRL  = (1<<0),
    KEY_MODIFIER_SHIFT = (1<<1),
    KEY_MODIFIER_ALT   = (1<<2)
};

static String
get_key_name(s32 index) {
    static char *strings[KEY_MAX] = {
#define Key(name, str) str,
#include "key_list.inc"
#undef Key
    };

    char *string = "INVALID";
    if (index >= 0 && index < KEY_MAX) {
        string = strings[index];
    }

    String result;
    result.str = string;
    result.size = (s32) strlen(string);

    return result;
}

#define KEY_INPUT_H
#endif
