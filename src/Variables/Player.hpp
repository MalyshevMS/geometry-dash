#pragma once
#include <string>
#include <vector>
#include "Cursor.hpp"

#define l_left -1
#define l_right 1

using BPair = std::pair <int, int>;

struct Player {
    int x = 0; // Player X
    int y = 0; // Player Y
    int spawn_x = 0; // Player X
    int spawn_y = 0; // Player Y
    int speed = 5;
    std::string current_anim = ""; // Player current animation
    Cursor cur = Cursor(); // Player cursor position
    bool noclip = false; // Player no clipping flag

    void update() {
        x += speed;
    }
};