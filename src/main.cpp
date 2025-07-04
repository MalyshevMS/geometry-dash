// #define debug
#define hitbox

#include <glad/glad.h> // OpenGL libs
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp> // glm libs
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/ShaderProgram.hpp" // my libs
#include "Renderer/Texture2D.hpp"
#include "Renderer/Sprite.hpp"
#include "Renderer/AnimatedSprite.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/TextureLoader.hpp"
#include "Resources/SpriteGroup.hpp"
#include "Resources/Parser.hpp"
#include "Other/KeyHandler.hpp"

#include "Variables/OpenGL.hpp" // variables
#include "Variables/Camera.hpp"
#include "Variables/Client.hpp"
#include "Variables/Player.hpp"
#include "Variables/Cursor.hpp"

#include "keys"

#include <iostream> // other libs
#include <string>
#include <cmath>
#include <vector>
#include <thread>

using namespace std;
using DPair = pair<string, uint64_t>;

OpenGL gl;
Camera cam;
Client cl;
Player pl;

ResourceManager rm_main; // Main Resource manager

TexLoader tl_main; // Main Texture loader

SprGroup sg_ground; // Group for ground
SprGroup sg_spikes; // Group for spikes
SprGroup sg_blocks; // Group for blocks
SprGroup sg_spikes_hbox; // Group for spikes hitbox
SprGroup sg_blocks_hbox; // Group for blocks hitbox
SprGroup sg_buttons; // Group for buttons
SprGroup sg_player; // Group for Player 1
SprGroup sg_player_hbox; // Group for Player hitbox
SprGroup sg_trail;

Parser pars_main; // Main parser

KeyHandler kh_main; // Main Key Handler

float __ticks;
float __ticks2;
float __buff_rot;
glm::vec2 __buff_fall;

/// @brief Function for resizing window
/// @param win GLFW window poiner
/// @param width new window width
/// @param height new window height
void sizeHandler(GLFWwindow* win, int width, int height) {
    gl.win_size.x = width;
    gl.win_size.y = height;
    glViewport(0, 0, gl.win_size.x, gl.win_size.y);
}

/// @brief Function for checking collisions beetween 2 sprite groups
/// @param sg1 First sprite group
/// @param sg2 Second sprite group
/// @return Is these groups colliding
// bool sg_collision(SprGroup& sg1, SprGroup& sg2) {
//     for (auto i : sg1.get_sprites()) {
//         for (auto j : sg2.get_sprites()) {
//             if (abs(i->getPos().x - j->getPos().x) <= gl.sprite_size && abs(i->getPos().y - j->getPos().y) <= gl.sprite_size) return true;
//         }
//     }
//     return false;
// }

auto parabola_fall = [](int x){
    return (1.f / 256.f) * x * x;
};

auto find_nearest = [](int x, int y){
    int remainder = x % y;
    if (remainder >= y / 2) {
        return x + (y - remainder);
    } else {
        return x - remainder;
    }
}; 

void fix_rotation() {
    pl.rotation = find_nearest(pl.rotation, 90);
    cl.is_fixed_rot = true;
}

void fix_clipping() {
    if (pl.y < 0) {
        pl.y++;
    }
}

void spawn_ground() {
    for(int i = 0; i < 18; i++) {
        sg_ground.add_sprite("Ground", "", gl.sprite_shader, 3 * gl.sprite_size, 3 * gl.sprite_size, 0.f, i * gl.sprite_size * 3 - 6 * gl.sprite_size, -240);
    }
}

void reset() {
    pl.x = pl.spawn_x;
    pl.y = pl.spawn_y;
    pl.rotation = 0.f;
    sg_ground.delete_all();
    spawn_ground();
}

bool spikes_collide() {
    for (auto i : sg_spikes_hbox.get_sprites()) {
        auto s_pos = i->getPos();
        auto p_pos = glm::vec2(pl.x, pl.y);
        auto s_size = cl.hbox_size;
        auto p_size = glm::vec2(gl.sprite_size);
        

        auto s_rt = s_pos + s_size;
        auto p_rt = p_pos + p_size;

        if (p_rt.x >= s_pos.x && p_rt.y >= s_pos.y && \
            p_pos.x <= s_rt.x && p_pos.y <= s_rt.y) return true;
    }
    return false;
}

/// @brief Checks if player collides floor
/// @param epsilon how much units player can go through sprite in Y direction
/// @param epsilon2 how much units player can be far away from sprite in X direction
/// @return true, if player collides floor. Else false
bool collides_floor(int epsilon = 2, int epsilon2 = 0) {
    if (pl.y <= epsilon) return true;
    vector sprites_pos = sg_blocks_hbox.get_sprites();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs(pl.y - sprites_pos[i]->getPos().y) <= gl.sprite_size - epsilon && pl.y - sprites_pos[i]->getPos().y >= epsilon && abs(pl.x - sprites_pos[i]->getPos().x) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

/// @brief Checks if player collides ceiling
/// @param epsilon how much units player can go through sprite in Y direction
/// @param epsilon2 how much units player can be far away from sprite in X direction
/// @return true, if player collides ceiling. Else false
bool collides_ceiling(int epsilon = 2 , int epsilon2 = 0) {
    vector sprites_pos = sg_blocks_hbox.get_sprites();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if ((pl.y + gl.sprite_size) - sprites_pos[i]->getPos().y >= -epsilon && (pl.y + gl.sprite_size) - sprites_pos[i]->getPos().y <= gl.sprite_size && abs(pl.x - sprites_pos[i]->getPos().x) < gl.sprite_size) return true;
    }
    return false;
}

/// @brief Checks if player collides left wall
/// @param epsilon how much units player can go through sprite in X direction
/// @param epsilon2 how much units player can be far away from sprite in Y direction
/// @return true, if player collides left wall. Else false
bool collides_left(int epsilon = 0, int epsilon2 = 2) {
    vector sprites_pos = sg_blocks_hbox.get_sprites();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (pl.x - (sprites_pos[i]->getPos().x + gl.sprite_size) <= -epsilon && pl.x - (sprites_pos[i]->getPos().x + gl.sprite_size) > -2 * gl.sprite_size && abs(pl.y - sprites_pos[i]->getPos().y) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

/// @brief Checks if player collides right wall
/// @param epsilon how much units player can go through sprite in X direction
/// @param epsilon2 how much units player can be far away from sprite in Y direction
/// @return true, if player collides right wall. Else false
bool collides_right(int epsilon = 0, int epsilon2 = 2) {
    vector sprites_pos = sg_blocks_hbox.get_sprites();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs((pl.x + gl.sprite_size) - sprites_pos[i]->getPos().x) <= -epsilon && abs(pl.y - sprites_pos[i]->getPos().y) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

void create_spike(int x, int y) {
    sg_spikes.add_sprite("Spike", "", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, x, y);
    sg_spikes_hbox.add_sprite("SpikeHitbox", "", gl.sprite_shader, cl.hbox_size.x, cl.hbox_size.y, 0.f, x + cl.hbox_size.x / 2, y);
}

void create_block(int x, int y) {
    sg_blocks.add_sprite("Block", "", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, x, y);
    sg_blocks_hbox.add_sprite("BlockHitbox", "", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, x, y);
}

void fall() {
    if (pl.y > 0 && !cl.is_jumping && !collides_floor()) {
        cl.is_falling = true;
        pl.y = -parabola_fall(pl.x - __buff_fall.x) + __buff_fall.y; // не спрашивай почему тут 31, оно работает, так что не трогай
        pl.rotation += -90.f / 40.f; // не спрашивай почему тут 32.f, оно работает, так что не трогай
        cl.is_fixed_rot = false;
    } else {
        cl.is_falling = false;
        __buff_fall.x = pl.x;
        __buff_fall.y = pl.y;
        if (!cl.is_fixed_rot) {
            fix_rotation();
        }
    }
}

void jump() {
    int end_point = pl.y + pl.jump_height;
    while (pl.y < end_point) {
        cl.is_jumping = true;
        pl.y += pl.jump_speed * (abs(pl.y - (end_point + 31)) / float(pl.jump_height)); // не спрашивай почему тут 31, оно работает, так что не трогай
        pl.rotation += -90.f / 32.f; // не спрашивай почему тут 32.f, оно работает, так что не трогай
        sleep(1);
    }
    cl.is_jumping = false;
}

void trail() {
    if (pl.x % 2 == 0) {
        sg_trail.add_sprite("Trail", "", gl.sprite_shader, 8, 8, pl.rotation, pl.x + 40, pl.y + 40);
    }

    if (sg_trail.get_sprites().size() >= 120 ) {
        sg_trail.delete_all();
    }
}

/// @brief Proceeds once key pressings (if key was hold down for a long it's still will be recognize as once pressing)
void onceKeyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) { 
    if (key == KEY_F && action == GLFW_PRESS) {
       cout << spikes_collide() << endl;
    }

    if (key == KEY_R && action == GLFW_PRESS) {
        reset();
    }

    if (key == KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }
}

/// @brief Function for proceeding keys pressing. P.S. This function supports long key pressing
/// @param win GLFW window pointer
void keyHandler(GLFWwindow* win) {
    if(glfwGetKey(win, KEY_ENTER) == GLFW_PRESS){
        cout << "X: " << pl.x << " Y: " << pl.y << " ROTATION: " << pl.rotation << endl;
    }

    if (glfwGetKey(win, KEY_UP) == GLFW_PRESS || glfwGetKey(win, KEY_SPACE) == GLFW_PRESS || glfwGetMouseButton(win, MOUSE_LEFT) == GLFW_PRESS) {
        if (!cl.is_jumping && !cl.is_falling) {
            thread t(jump);
            t.detach();
        }
    }
}

int main(int argc, char const *argv[]) {
    size_t found = string(argv[0]).find_last_of("/\\");
    string exe_path = string(argv[0]).substr(0, found + 1);

    std::fstream f;
    f.open(exe_path + "config.json");
    nlohmann::json file;
    f >> file;

    // Engine vars
    auto _gl = file["OpenGL"];
    gl.sprite_shader = _gl["sprite.shader"];
    gl.sprite_shader_vertex = _gl["sprite.shader.vertex"];
    gl.sprite_shader_fragment = _gl["sprite.shader.fragment"];
    gl.sprite_size = 80;
    gl.font_width = _gl["font.width"];
    gl.font_height = _gl["font.height"];
    gl.win_size.x = _gl["window.width"];
    gl.win_size.y = _gl["window.height"];
    gl.win_title = _gl["window.title"];
    gl.fullscreen = _gl["fullscreen"];

    auto _cam = file["Camera"];
    cam.x = pl.x - gl.win_size.x / 2;
    cam.y = pl.y - gl.sprite_size;
    cam.rot = 180.f;
    cam.mag = 1.f;
    cam.speed = _cam["speed"];
    cam.mag_speed = _cam["mag.speed"];
    cam.locked = _cam["locked"];

    auto _pl = file["Player"];
    pl.x = _pl["spawn.x"];
    pl.y = _pl["spawn.y"];
    pl.spawn_x = _pl["spawn.x"];
    pl.spawn_y = _pl["spawn.y"];
    pl.current_anim = "";
    pl.noclip = _pl["noclip"];

    if (!glfwInit()) { // Check for GLFW
        cerr << "glfwInit failed!" << endl;
        return -1;
    }

    // GLFW window settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, 0);

    GLFWwindow* window;
    if (gl.fullscreen) { // Creating window (fullscreen or windowed)
        window = glfwCreateWindow(gl.win_size.x, gl.win_size.y, gl.win_title.c_str(), glfwGetPrimaryMonitor(), nullptr);
    } else {
        window = glfwCreateWindow(gl.win_size.x, gl.win_size.y, gl.win_title.c_str(), nullptr, nullptr);
    }

    if (!window) { // Checking for creating window
        cerr << "glfwCreateWindow failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, onceKeyHandler); // Setting Key Handler
    glfwSetWindowSizeCallback(window, sizeHandler); // Setting Size Handler

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) { // Checking for GLAD
        cerr << "Can't load GLAD!" << endl;
    }

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl; // Displaying OpenGL info
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    glClearColor(0.19f, 0.61f, 0.61f, 1.f); // Sky color (in vec4 format)

    {
        rm_main = ResourceManager(argv[0]); // Binding all classes together
        tl_main = TexLoader(&rm_main);
        sg_ground = SprGroup(&rm_main);
        sg_player = SprGroup(&rm_main);
        sg_buttons = SprGroup(&rm_main);
        sg_trail = SprGroup(&rm_main);
        sg_spikes = SprGroup(&rm_main);
        sg_blocks = SprGroup(&rm_main);
        sg_spikes_hbox = SprGroup(&rm_main);
        sg_blocks_hbox = SprGroup(&rm_main);
        sg_player_hbox = SprGroup(&rm_main);
        pars_main = Parser(&rm_main, &tl_main, &sg_ground, &gl);
        kh_main = KeyHandler(window);

        // Creating and checking for sprite shader
        auto spriteShaderProgram = rm_main.loadShaders(gl.sprite_shader, gl.sprite_shader_vertex, gl.sprite_shader_fragment);
        if (!spriteShaderProgram) {
            cerr << "Can't create SpriteShader" << endl;
            return -1;
        }

        // Using sprite shader
        spriteShaderProgram->use();
        spriteShaderProgram->setInt("tex", 0);

        pl.current_anim = "";

        vector <string> chars;
        for (int i = 32; i < 96; i++) {
            string str;
            str.push_back((char)i);
            chars.push_back(str);
        }
        tl_main.add_textures_from_atlas("Font", "res/textures/font.png", chars, glm::vec2(64, 64));
        tl_main.add_texture("Filler", "res/textures/filler.png");
        tl_main.add_texture("Player", "res/textures/player.png");
        tl_main.add_texture("Ground", "res/textures/ground.png");
        tl_main.add_texture("Block", "res/textures/block.png");
        tl_main.add_texture("Trail", "res/textures/trail.png");
        tl_main.add_texture("Spike", "res/textures/spike.png");
        tl_main.add_texture("SpikeHitbox", "res/textures/hitbox_spike.png");
        tl_main.add_texture("BlockHitbox", "res/textures/hitbox_block.png");

        sg_player.set_timer();

        cam.y = pl.y - gl.win_size.y / 2 + 200;

        sg_player.add_sprite("Player", "", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, pl.x, pl.y);
        spawn_ground();

        create_spike(gl.sprite_size * 20, 0);
        create_spike(gl.sprite_size * 40, 0);

        create_block(gl.sprite_size * 30, gl.sprite_size);
        create_block(gl.sprite_size * 31, gl.sprite_size);
        create_block(gl.sprite_size * 32, gl.sprite_size);
        create_block(gl.sprite_size * 32, gl.sprite_size * 2);
        create_block(gl.sprite_size * 32, gl.sprite_size * 3);

        sg_player_hbox.add_sprite("SpikeHitbox", "", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, pl.x, pl.y);

        #ifndef hitbox
            sg_spikes_hbox.hide_all();
        #endif

        float projMat_right  = 0.f;
        float projMat_top    = 0.f;
        float projMat_left   = 0.f;
        float projMat_bottom = 0.f;
        glm::mat4 projMat;
        double cx, cy;

        while (!glfwWindowShouldClose(window)) { // Main game loop
            glClear(GL_COLOR_BUFFER_BIT);

            keyHandler(window); // Setting (old) key handler
            kh_main.use(cl); // Setting (new) key handler

            // Projection matrix variables
            projMat_right  = gl.win_size.x * cam.mag + cam.x;
            projMat_top    = gl.win_size.y * cam.mag + cam.y;
            projMat_left   = - gl.win_size.x * (cam.mag - 1) + cam.x;
            projMat_bottom = - gl.win_size.y * (cam.mag - 1) + cam.y;

            projMat = glm::ortho(projMat_left, projMat_right, projMat_bottom, projMat_top, -100.f, 100.f); // Setting projection matrix

            // Using projection matrix
            spriteShaderProgram->setMat4("projMat", projMat);

            tl_main.bind_all(); // Binding all textures
            
            fall();
            fix_clipping();
            if (spikes_collide()) {
                cout << "TYLYP" << endl;
                reset();
            }
            #ifdef debug
                trail();
            #endif

            pl.update();
            sg_player.set_pos(pl.x, pl.y);
            sg_player_hbox.set_pos(pl.x, pl.y);
            sg_player.rotate_all(pl.rotation);
            cam.x = pl.x - gl.win_size.x / 2 + 240;

            if (pl.x % (27 * gl.sprite_size) == 0 && pl.x != 0) {
                sg_ground.move_all(27 * gl.sprite_size, 0);
            }

            sg_player.update_all();

            // Rendering all sprites
            sg_player.render_all();
            sg_player_hbox.render_all();
            sg_ground.render_all();
            sg_buttons.render_all();
            sg_trail.render_all();
            sg_spikes.render_all();
            sg_blocks.render_all();
            sg_spikes_hbox.render_all();
            sg_blocks_hbox.render_all();
            
            glfwGetCursorPos(window, &cx, &cy);
            pl.cur.x = cx + cam.x;
            pl.cur.y = gl.win_size.y - cy + cam.y;

            sleep(1); // 1ms delay
            glfwSwapBuffers(window); // Swapping front and back buffers
            glfwPollEvents(); // Polling events
        }
        
        // Deleting all sprites from all groups
        sg_spikes.render_all();
        sg_spikes_hbox.render_all();
        sg_blocks.render_all();
        sg_blocks_hbox.render_all();
        sg_ground.delete_all();
        sg_trail.delete_all();
    }


    glfwTerminate(); 
    return 0; // Exiting programm
}