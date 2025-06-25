// #define debug
#define online

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

SprGroup sg_sprites; // Group for obstacles, walls, etc.
SprGroup sg_buttons; // Group for buttons
SprGroup sg_player; // Group for Player 1

Parser pars_main; // Main parser

KeyHandler kh_main; // Main Key Handler

float __ticks;
float __ticks2;

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
bool sg_collision(SprGroup& sg1, SprGroup& sg2) {
    for (auto i : sg1.get_sprites()) {
        for (auto j : sg2.get_sprites()) {
            if (abs(i->getPos().x - j->getPos().x) <= gl.sprite_size && abs(i->getPos().y - j->getPos().y) <= gl.sprite_size) return true;
        }
    }
    return false;
}

void jump() {
    int end_point = pl.y + pl.jump_height;
    while (pl.y < end_point) {
        pl.y += pl.jump_speed * (abs(pl.y - (end_point + 31)) / float(pl.jump_height)); // не спрашивай почему тут 31, оно работает, так что не трогай
        pl.rotation += -90.f / 72.f; // не спрашивай почему тут 72.f, оно работает, так что не трогай
        sleep(1);
    }
}

/// @brief Proceeds once key pressings (if key was hold down for a long it's still will be recognize as once pressing)
void onceKeyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) { 
    if (key == KEY_UP && action == GLFW_PRESS) {
       thread t(jump);
        t.detach();
        cout << "JUMP" << endl;
    }

    if (key == KEY_R && action == GLFW_PRESS) {
        pl.y = pl.spawn_y;
        cout << "RESET" << endl;
    }

    if (key == KEY_ESCAPE && action == GLFW_PRESS) {
        cout << "EXIT" << endl;
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }
}

/// @brief Function for proceeding keys pressing. P.S. This function supports long key pressing
/// @param win GLFW window pointer
void keyHandler(GLFWwindow* win) {
    if(glfwGetKey(win, KEY_ENTER) == GLFW_PRESS){
        cout << "X: " << pl.x << " Y: " << pl.y << " ROTATION: " << pl.rotation << endl;
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

    string com;

    #ifdef debug // Debugger console
        do {
            cout << "Parcour-game: ";
            getline(cin, com);

            if (com == "$exit") return 0;

            cmd.action(com, &pl);
        } while (com != "$play");
    #endif

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
        sg_sprites = SprGroup(&rm_main);
        sg_player = SprGroup(&rm_main);
        sg_buttons = SprGroup(&rm_main);
        pars_main = Parser(&rm_main, &tl_main, &sg_sprites, &gl);
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

        sg_player.set_timer();

        cam.y = pl.y - gl.win_size.y / 2 + 200;

        sg_player.add_sprite("Player", "", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, pl.x, pl.y);
        for(int i = 0; i < 18; i++) {
            sg_sprites.add_sprite("Ground", "", gl.sprite_shader, 3 * gl.sprite_size, 3 * gl.sprite_size, 0.f, i * gl.sprite_size * 3 - 6 * gl.sprite_size, -240);
        }

        while (!glfwWindowShouldClose(window)) { // Main game loop
            glClear(GL_COLOR_BUFFER_BIT);

            keyHandler(window); // Setting (old) key handler
            kh_main.use(cl); // Setting (new) key handler

            // Projection matrix variables
            float projMat_right  = gl.win_size.x * cam.mag + cam.x;
            float projMat_top    = gl.win_size.y * cam.mag + cam.y;
            float projMat_left   = - gl.win_size.x * (cam.mag - 1) + cam.x;
            float projMat_bottom = - gl.win_size.y * (cam.mag - 1) + cam.y;

            glm::mat4 projMat = glm::ortho(projMat_left, projMat_right, projMat_bottom, projMat_top, -100.f, 100.f); // Setting projection matrix

            // Using projection matrix
            spriteShaderProgram->setMat4("projMat", projMat);

            tl_main.bind_all(); // Binding all textures

            pl.update();
            sg_player.set_pos(pl.x, pl.y);
            sg_player.rotate_all(pl.rotation);
            cam.x = pl.x - gl.win_size.x / 2 + 240;

            if (pl.x % (27 * gl.sprite_size) == 0 && pl.x != 0) {
                sg_sprites.move_all(27 * gl.sprite_size, 0);
            }

            sg_player.update_all();

            // Rendering all sprites
            sg_player.render_all();
            sg_sprites.render_all();
            sg_buttons.render_all();
            
            double cx, cy;
            glfwGetCursorPos(window, &cx, &cy);
            pl.cur.x = cx + cam.x;
            pl.cur.y = gl.win_size.y - cy + cam.y;

            sleep(1); // 1ms delay
            glfwSwapBuffers(window); // Swapping front and back buffers
            glfwPollEvents(); // Polling events
        }
        
        // Deleting all sprites from all groups
        sg_sprites.delete_all();
    }


    glfwTerminate(); 
    return 0; // Exiting programm
}