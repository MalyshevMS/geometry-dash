#pragma once

#include <glad/glad.h>
#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture2D.hpp"
#include "ShaderProgram.hpp"

#define FLAG_NOCOLOR    0x00000001
#define FLAG_NOALPHA    0x00000002
#define FLAG_NOMOVE     0x00000003
#define FLAG_NOROTATE   0x00000004

namespace Renderer  {
    class Sprite {
    protected:
        std::shared_ptr<Texture2D> _tex;
        std::shared_ptr<ShaderProgram> _shader_prog;
        std::vector<int> _groups;
        std::vector<std::pair<int, bool>> _flags;
        glm::vec2 _pos, _size;
        Color _color = Color(1.f);
        float _rotation;
        float _zLayer = 0.f;
        float _alpha = 1.f;
        GLuint _VAO;
        GLuint _vertexCoordsVBO;
        GLuint _texCoordsVBO;
    public:
        bool _render = true;
        Sprite(const std::shared_ptr<Texture2D>& tex, const std::string& subtexture, const std::shared_ptr<ShaderProgram>& shader_prog, const glm::vec2& pos, const glm::vec2& size, const float& rotation) {
            _tex = std::move(tex);
            _shader_prog = std::move(shader_prog);
            _size = size;
            _pos = pos;
            _rotation = rotation;

            const GLfloat vertCoords[] = {
                0, 0,
                0, 1,
                1, 1,

                1, 1,
                1, 0,
                0, 0
            };

            auto subtex = _tex->get_subtex(std::move(subtexture));

            const GLfloat texCoords[] {
                subtex.LB.x, subtex.LB.y,
                subtex.LB.x, subtex.RT.y,
                subtex.RT.x, subtex.RT.y,

                subtex.RT.x, subtex.RT.y,
                subtex.RT.x, subtex.LB.y,
                subtex.LB.x, subtex.LB.y
            };

            glEnable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_TRUE);

            glGenVertexArrays(1, &_VAO);
            glBindVertexArray(_VAO);

            glGenBuffers(1, &_vertexCoordsVBO);
            glBindBuffer(GL_ARRAY_BUFFER, _vertexCoordsVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertCoords), &vertCoords, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            glGenBuffers(1, &_texCoordsVBO);
            glBindBuffer(GL_ARRAY_BUFFER, _texCoordsVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), &texCoords, GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        };

        ~Sprite() {
            glDeleteBuffers(1, &_vertexCoordsVBO);
            glDeleteBuffers(1, &_texCoordsVBO);

            glDeleteVertexArrays(1, &_VAO);
        };

        Sprite(const Sprite&) = delete;
        Sprite& operator=(const Sprite&) = delete;

        virtual void render() const {
            if (!_render) return;
            
            _shader_prog->use();

            glm::mat4 model(1.f);

            _shader_prog->setFloat("zLayer", _zLayer);
            _shader_prog->setFloat("u_alpha", _alpha);
            _shader_prog->setVec3("spriteColor", _color);

            model = glm::translate(model, glm::vec3(_pos, 0.f));
            model = glm::translate(model, glm::vec3(0.5f * _size.x, 0.5f * _size.y, 0.f));
            model = glm::rotate(model, glm::radians(_rotation), glm::vec3(0.f, 0.f, 1.f));
            model = glm::translate(model, glm::vec3(-0.5f * _size.x, -0.5f * _size.y, 0.f));
            model = glm::scale(model, glm::vec3(_size, 1.f));

            glBindVertexArray(_VAO);
            _shader_prog->setMat4("modelMat", model);

            glActiveTexture(GL_TEXTURE0);
            _tex->bind();

            glDrawArrays(GL_TRIANGLES, 0 ,6);
            glBindVertexArray(0);
        };

        void move(const glm::vec2& xy) {
            setPos(_pos + xy);
        };

        void setPos(const glm::vec2& pos) {
            _pos = pos;
        };

        glm::vec2 getPos() { return _pos; };

        void setSize(glm::vec2& size) {
            _size = size;
        };

        glm::vec2 getSize() { return _size; };

        void setRotation(const float rotation) {
            _rotation = rotation;
        };

        float getRotation() { return _rotation; };

        void setZLayer(const float zLayer) {
            _zLayer = zLayer;
        }

        float getZLayer() { return _zLayer; };

        void setAlpha(const float alpha) {
            if (alpha >= 0.f && alpha <= 1.f) {
                _alpha = alpha;
            } else {
                std::cerr << "Alpha must be between 0.f and 1.f" << std::endl;
            }
        }

        float getAlpha() { return _alpha; }

        void setColor(const Color& color) {
            if (color.x >= 0.f && color.x <= 1.f && \
                color.y >= 0.f && color.y <= 1.f && \
                color.z >= 0.f && color.z <= 1.f) {
                _color = color;
            } else {
                std::cerr << "Color must be between 0.f and 1.f" << std::endl;
            }
        }

        Color getColor() { return _color; }

        void addGroup(int group) {
            for (auto& i : _groups) {
                if (i == group) return;
            }
            _groups.push_back(group);
        }

        void removeGroup(int group) {
            for (auto it = _groups.begin(); it != _groups.end(); it++) {
                if (*it == group) {
                    _groups.erase(it);
                    return;
                }
            }
        }

            void addFlag(int flag, bool value) {
                for (auto& i : _flags) {
                    if (i.first == flag) {
                        i.second = value;
                        return;
                    }
                }
                _flags.push_back(std::make_pair(flag, value));
            }

            bool getFlag(int flag) {
                if (_flags.size() == 0) return false;
                for (auto& i : _flags) {
                    if (i.first == flag) return i.second;
                }
                return false;
            }
    };
}