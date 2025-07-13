#pragma once

#include "SpriteGroup.hpp"

class Level;

class SubGroup {
protected:
    int id;
    std::vector<std::shared_ptr<Renderer::AnimatedSprite>> sprites;
public:
    friend class Level;
    SubGroup(int id) : id(id) {};

    void add_sprite(std::shared_ptr<Renderer::AnimatedSprite> spr) {
        sprites.push_back(spr);
        spr->addGroup(id);
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
};