#pragma once

#include "SpriteGroup.hpp"

class Level;

struct SpriteState {
    float alpha;
    Color color;
    glm::vec2 pos;
    float rotation;
    SpriteState(float alpha, Color color, glm::vec2 pos, float rotation) {
        this->alpha = alpha;
        this->color = color;
        this->pos = pos;
        this->rotation = rotation;
    }
};

class SubGroup {
protected:
    int id;
    bool locked = false;
    std::vector<std::shared_ptr<Renderer::AnimatedSprite>> sprites;
    std::vector<SpriteState> default_states;
public:
    friend class Level;
    SubGroup(int id) : id(id) {};

    void add_sprite(std::shared_ptr<Renderer::AnimatedSprite> spr) {
        if (!locked) {
            sprites.push_back(spr);
            spr->addGroup(id);
        }
    }

    void lock() {
        for (int i = 0; i < sprites.size(); i++) {
            auto spr = sprites[i];
            default_states.push_back(SpriteState(spr->getAlpha(), spr->getColor(), spr->getPos(), spr->getRotation()));
        }
        locked = true;
    }

    void reset() { // В тихом омуте черти водятся
        for (int i = 0; i < default_states.size(); i++) {
            sprites[i]->setAlpha    (default_states[i].alpha      );
            sprites[i]->setColor    (default_states[i].color      );
            sprites[i]->setPos      (default_states[i].pos        );
            sprites[i]->setRotation (default_states[i].rotation   );
        }
    }

    void setAlpha(float alpha) {
        for (auto& spr : sprites) {
            if(spr->getFlag(FLAG_NOALPHA)) continue;
            spr->setAlpha(alpha);
        }
    }

    void setColor(const Color& color) {
        for (auto& spr : sprites) {
            if(spr->getFlag(FLAG_NOCOLOR)) continue;
            spr->setColor(color);
        }
    }

    void move(int x, int y) {
        for (auto& spr : sprites) {
            if(spr->getFlag(FLAG_NOMOVE)) continue;
            spr->move(glm::vec2(x, y));
        }
    }

    void rotate(float rot) {
        for (auto& spr : sprites) {
            if(spr->getFlag(FLAG_NOROTATE)) continue;
            spr->setRotation(rot);
        }
    }
};

class Level {
private:
    std::vector<SubGroup> groups;
public:
    Level() {};
    void add_group(int id) {
        for (auto& i : groups) {
            if (i.id == id) {
                return;
            }
        }
        groups.push_back(SubGroup(id));
    }

    SubGroup& group(int id) {
        for (auto& i : groups) {
            if (i.id == id) return i;
        }
        add_group(id);
        return groups[groups.size() - 1];
    }

    SubGroup& operator[](const int& id) {
        return group(std::move(id));
    }

    std::vector<int> group_list() {
        std::vector<int> result;
        for (auto i : groups) {
            result.push_back(i.id);
        }
        return result;
    }
};