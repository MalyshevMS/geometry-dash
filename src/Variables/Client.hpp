#pragma once

#include <string>
#include <glm/vec2.hpp>

struct Client {
    bool in_game;
    bool is_jumping;
    bool is_falling;
    bool is_fixed_rot = true;
    glm::vec2 hbox_size = glm::vec2(40, 60);
};