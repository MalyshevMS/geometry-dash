#pragma once

#include "../../Resources/SubGroup.hpp"
#include "../../Variables/Player.hpp"
#include "../../defines"

class Trigger {
protected:
    int x, y;
    bool spawn, touch;
    bool activated = false;
    bool multi;
    std::vector<int> groups;
    Level* lvl;
public:
    Trigger(int x, int y, Level* lvl) {
        this->x = x;
        this->y = y;
        this->lvl = lvl;
    };

    virtual void action() {
        activated = true;
    }

    virtual void update(Player& pl) {
        if (!spawn) {
            if (pl.x >= x && !activated) {
                action();
            }
        }

        if (touch) {
            auto b_pos = glm::vec2(x, y);
            auto p_pos = glm::vec2(pl.x, pl.y);
            auto b_size = glm::vec2(1.b);
            auto p_size = glm::vec2(1.b);
            

            auto b_rt = b_pos + b_size;
            auto p_rt = p_pos + p_size;

            if (p_rt.x >= b_pos.x && p_rt.y >= b_pos.y && \
                p_pos.x <= b_rt.x && p_pos.y <= b_rt.y) action();
        }
    }

    void addGroup(int group) {
        for (auto& i : this->groups) {
            if (i == group) return;
        }
        this->groups.push_back(group);
    }

    void removeGroup(int group) {
        for (auto it = this->groups.begin(); it != this->groups.end(); it++) {
            if (*it == group) {
                this->groups.erase(it);
                return;
            }
        }
    }
};