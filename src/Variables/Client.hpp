#pragma once

#include <string>
#include <glm/vec2.hpp>

struct Client {
    bool _debug = false;
    bool in_game;
    bool is_jumping;
    bool is_falling;
    bool is_fixed_rot = true;
    bool show_hitboxes = false;
    glm::vec2 hbox_size = glm::vec2(40, 60);
    int end_pos = 0;
    int fps = 60;
    int attempt = 0;
};