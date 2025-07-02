#pragma once

#include <string>

struct Client {
    bool in_game;
    bool is_jumping;
    bool is_falling;
    bool is_fixed_rot = true;
};