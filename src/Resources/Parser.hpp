#include "ResourceManager.hpp"
#include "SpriteGroup.hpp"
#include "SubGroup.hpp"
#include "TextureLoader.hpp"
#include "../Variables/OpenGL.hpp"
#include "../Variables/Client.hpp"
#include "json.hpp"
#include "../defines"


class Parser {
private:
    using json = nlohmann::json;
    using DPair = std::pair<std::string, uint64_t>;
    ResourceManager* rm = nullptr;
    TexLoader* tx = nullptr;
    SprGroup* sg_spikes = nullptr;
    SprGroup* sg_spikes_hbox = nullptr;
    SprGroup* sg_blocks = nullptr;
    SprGroup* sg_blocks_hbox = nullptr;
    Level* lvl = nullptr;
    OpenGL* gl = nullptr;
    Client* cl = nullptr;
public:
    void create_spike(int x, int y, const std::vector<int>& groups) {
        sg_spikes->add_sprite("Spike", "", gl->sprite_shader, 1.b, 1.b, 0.f, x, y);
        sg_spikes_hbox->add_sprite("SpikeHitbox", "", gl->sprite_shader, cl->hbox_size.x, cl->hbox_size.y, 0.f, x + cl->hbox_size.x / 2, y);
        if (groups.size() > 0) {
            for (auto i : groups) {
                lvl->group(i).add_sprite((*sg_spikes)[sg_spikes->get_sprites().size() - 1]);
                lvl->group(i).add_sprite((*sg_spikes_hbox)[sg_spikes_hbox->get_sprites().size() - 1]);
            }
        }
    }

    void create_block(int x, int y, const std::vector<int>& groups) {
        sg_blocks->add_sprite("Block", "", gl->sprite_shader, 1.b, 1.b, 0.f, x, y);
        sg_blocks_hbox->add_sprite("BlockHitbox", "", gl->sprite_shader, 1.b, 1.b, 0.f, x, y);
        if (groups.size() > 0) {
            for (auto i : groups) {
                lvl->group(i).add_sprite((*sg_blocks)[sg_blocks->get_sprites().size() - 1]);
                lvl->group(i).add_sprite((*sg_blocks_hbox)[sg_blocks_hbox->get_sprites().size() - 1]);
            }
        }
    }

    json get_json(const std::string& path) {
        std::fstream f;
        f.open(rm->getExePath() + path);
        json file;
        f >> file;
        return file;
    }

    Parser(ResourceManager* rm = nullptr, TexLoader* tx = nullptr,            \
           SprGroup* sg_spikes = nullptr, SprGroup* sg_spikes_hbox = nullptr, \
           SprGroup* sg_blocks = nullptr, SprGroup* sg_blocks_hbox = nullptr, \
           Client* cl = nullptr, OpenGL* gl = nullptr, Level* lvl = nullptr) {
        this->rm = rm;
        this->tx = tx;
        this->gl = gl;
        this->cl = cl;
        this->sg_blocks = sg_blocks;
        this->sg_spikes = sg_spikes;
        this->sg_blocks_hbox = sg_blocks_hbox;
        this->sg_spikes_hbox = sg_spikes_hbox;
        this->lvl = lvl;
    };

    void parse_player(std::string path, SprGroup* sg_p, const glm::vec2& spawn_pos) {
        json file = get_json(std::move(path));

        std::vector <json> atl_arr = file["textures"]["atlas"];
        for (auto i : atl_arr) {
            tx->add_textures_from_atlas(i["name"], i["path"], i["subtexs"], glm::vec2(i["size.x"], i["size.y"]));
        }

        sg_p->add_sprite("Player", "default", gl->sprite_shader, gl->sprite_size, gl->sprite_size, 0.f, spawn_pos.x, spawn_pos.y);

        std::vector <json> anims = file["animations"];
        for (auto i : anims) {
            std::vector <DPair> result;
            std::vector <json> playback = i["playback"];
            for (auto j : playback) {
                result.push_back(DPair(j["tex"], j["duration"]));
            }
            sg_p->add_animation(0, i["name"], result);
        }
    }

    void parse_lvl(std::string path) {
        json file = get_json(std::move(path));
        std::vector <json> lvl = file["level"];

        sg_blocks_hbox->set_flag_all(FLAG_NOALPHA, true);
        sg_blocks_hbox->set_flag_all(FLAG_NOCOLOR, true);
        sg_blocks_hbox->set_flag_all(FLAG_NOROTATE, true);

        sg_spikes_hbox->set_flag_all(FLAG_NOALPHA, true);
        sg_spikes_hbox->set_flag_all(FLAG_NOCOLOR, true);
        sg_spikes_hbox->set_flag_all(FLAG_NOROTATE, true);

        for(int i =0; i< lvl.size(); i++){
            json _elem = lvl[i];
            std::vector<int> groups = _elem["groups"];
            if ( _elem["type"] == "spike"){
                create_spike(int(_elem["x"]) * 1.b , int(_elem["y"]) * 1.b, groups);
            }
            else if( _elem["type"] == "block"){
                create_block(int(_elem["x"]) * 1.b, int(_elem["y"]) * 1.b, groups);
            }
        }
    };
};