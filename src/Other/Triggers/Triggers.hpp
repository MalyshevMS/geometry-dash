#include "Trigger.hpp"

namespace Triggers {
    class Alpha : public Trigger { // Alpha trigger
    private:
        int target;
        float opacity;
    public:
        Alpha(int target, float opacity, int x, int y, Level* lvl) : Trigger(x, y, lvl), \
                                                                     target(target), opacity(opacity) {};
        void action() override {
            lvl->group(target).setAlpha(opacity);
            Trigger::action();
        }
    };

    class ColorGroup : public Trigger { // Color trigger (for groups)
    private:
        int target;
        Color color;
    public:
        ColorGroup(int target, Color color, int x, int y, Level* lvl) : Trigger(x, y, lvl), \
                                                                     target(target), color(color) {};
        void action() override {
            lvl->group(target).setColor(color);
            Trigger::action();
        }
    };

    class Move : public Trigger { // Move trigger
    private:
        int target;
        int diff_x;
        int diff_y;
    public:
        Move(int target, int diff_x, int diff_y, int x, int y, Level* lvl) : Trigger(x, y, lvl), \
                                                                target(target), diff_x(diff_x), diff_y(diff_y) {};
        void action() override {
            lvl->group(target).move(diff_x, diff_y);
            Trigger::action();
        }
    };

    class Rotate : public Trigger { // Rotate trigger
    private:
        int target;
        float rotation;
    public:
        Rotate(int target, float rotation, int x, int y, Level* lvl) : Trigger(x, y, lvl), \
                                                                     target(target), rotation(rotation) {};
        void action() override {
            lvl->group(target).rotate(rotation);
            Trigger::action();
        }
    };
};