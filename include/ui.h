// MIT License
// 
// Copyright (c) 2021 Stefano Allegretti, Davide Papazzoni, Nicola Baldini, Lorenzo Governatori e Simone Gemelli
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#if !defined NIKMAN_UI_H
#define NIKMAN_UI_H

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <optional>

#include "utility.h"
#include "shader.h"


struct Glyph {

    char c;
    float w_space;
    float ox;
    float oy;
    float x;
    float y;
    float w;
    float h;
    std::vector<std::pair<char, int>> kernings;

};


struct Font {

    std::string family;
    int size;
    int h_space;
    Glyph glyphs[95];
    unsigned int texture;
    int texture_width;
    int texture_height;

    Font() {}

    Font(const Font& other) = delete;
    Font(Font&& other) = delete;
    Font& operator=(const Font& other) = delete;
    
    Font& operator=(Font&& other) {
        glDeleteTextures(1, &texture);        
        texture = other.texture;

        family = std::move(other.family);
        size = other.size;
        texture_width = other.texture_width;
        texture_height = other.texture_height;
        h_space = other.h_space;
        for (int i = 0; i < 95; ++i) {
            glyphs[i] = std::move(other.glyphs[i]);
        }

        return *this;
    }

    ~Font() {
        glDeleteTextures(1, &texture);
    };

};


bool ReadFont(const char* filename, Font& font) {

    font = Font();

    // Very dirty and specific xml parser
    std::ifstream is(filename);
    if (!is.is_open()) {
        std::cerr << "Error in ReadFont: can't open font description.\n";
        return false;
    }

    char c;

#define READ_UNTIL(ch) { c = 0; while (c != (ch)) { is >> c; } }

    READ_UNTIL('>');
    READ_UNTIL('"');

    is >> font.size;
    is >> c;
    READ_UNTIL('"');
    is >> font.family;
    font.family.resize(font.family.size() - 1);

    READ_UNTIL('"');
    is >> font.h_space;
    READ_UNTIL('<');

    for (int i = 0; i < 95; ++i) {

        READ_UNTIL('"');
        font.glyphs[i].c = i + 32;
        is >> font.glyphs[i].w_space;
        is >> c;
        READ_UNTIL('"');
        is >> font.glyphs[i].ox;
        is >> font.glyphs[i].oy;
        is >> c;
        READ_UNTIL('"');
        is >> font.glyphs[i].x;
        is >> font.glyphs[i].y;
        is >> font.glyphs[i].w;
        is >> font.glyphs[i].h;
        is >> c;
        READ_UNTIL('"');
        READ_UNTIL('"');
        is >> c;
        if (c == '/') {
            continue;
        }

        while (true) {
            READ_UNTIL('<');
            is >> c;
            if (c == '/') {
                break;
            }
            READ_UNTIL('"');
            int advance;
            is >> advance;
            is >> c;
            READ_UNTIL('"');
            char next;
            is >> next;
            font.glyphs[i].kernings.emplace_back(next, advance);
        }

    }

    // Texture
    stbi_set_flip_vertically_on_load(false);
    font.texture = MakeFontTexture("centaur_regular_32.png", font.texture_width, font.texture_height, false, true);
    stbi_set_flip_vertically_on_load(true);
    return true;

#undef READ_UNTIL
}


std::vector<float> GetStringVertices(const char* str, const Font& font) {
    int len = strlen(str);
    std::vector<float> vertices(strlen(str) * 24);
    float current_x = 0;
    float current_y = 0;
    for (int i = 0; i < len; ++i) {
        const Glyph& g = font.glyphs[str[i] - 32];
        vertices[i * 24 + 0] = current_x + g.ox;
        vertices[i * 24 + 1] = current_y - g.oy;
        vertices[i * 24 + 2] = g.x;
        vertices[i * 24 + 3] = g.y;
        vertices[i * 24 + 4] = current_x + g.ox + g.w;
        vertices[i * 24 + 5] = current_y - g.oy;
        vertices[i * 24 + 6] = g.x + g.w;
        vertices[i * 24 + 7] = g.y;
        vertices[i * 24 + 8] = current_x + g.ox;
        vertices[i * 24 + 9] = current_y - g.oy - g.h;
        vertices[i * 24 + 10] = g.x;
        vertices[i * 24 + 11] = g.y + g.h;
        vertices[i * 24 + 12] = current_x + g.ox + g.w;
        vertices[i * 24 + 13] = current_y - g.oy;
        vertices[i * 24 + 14] = g.x + g.w;
        vertices[i * 24 + 15] = g.y;
        vertices[i * 24 + 16] = current_x + g.ox;
        vertices[i * 24 + 17] = current_y - g.oy - g.h;
        vertices[i * 24 + 18] = g.x;
        vertices[i * 24 + 19] = g.y + g.h;
        vertices[i * 24 + 20] = current_x + g.ox + g.w;
        vertices[i * 24 + 21] = current_y - g.oy - g.h;
        vertices[i * 24 + 22] = g.x + g.w;
        vertices[i * 24 + 23] = g.y + g.h;

        current_x += g.w_space;

        // Consider kernings
        const char next = str[i + 1];
        for (const auto& x : g.kernings) {
            if (x.first == next) {
                current_x += x.second;
                break;
            }
        }

    }

    for (int i = 0; i < len * 24; ) {
        i += 2;
        vertices[i++] /= font.texture_width;
        vertices[i++] /= font.texture_height;
    }

    return vertices;
}

struct Writing {

    static const glm::vec3 normal_color;
    static const glm::vec3 highlighted_color;

    unsigned int VAO;
    unsigned int VBO;
    float x;
    float y;
    unsigned int n_vertices;
    bool highlighted;
    const bool dynamic;
    const Font& font;

    Writing(const char* str, int x_, int y_, const Font& font_, bool dynamic_ = false, bool highlighted_ = false) :
        x(x_), 
        y(y_), 
        dynamic(dynamic_),
        highlighted(highlighted_),
        font(font_)
    {

        int len = strlen(str);
        std::vector<float> vertices(strlen(str) * 24);
        float current_x = 0;
        float current_y = 0;
        for (int i = 0; i < len; ++i) {
            const Glyph& g = font.glyphs[str[i] - 32];
            vertices[i * 24 + 0] = current_x + g.ox;
            vertices[i * 24 + 1] = current_y - g.oy;
            vertices[i * 24 + 2] = g.x;
            vertices[i * 24 + 3] = g.y;
            vertices[i * 24 + 4] = current_x + g.ox + g.w;
            vertices[i * 24 + 5] = current_y - g.oy;
            vertices[i * 24 + 6] = g.x + g.w;
            vertices[i * 24 + 7] = g.y;
            vertices[i * 24 + 8] = current_x + g.ox;
            vertices[i * 24 + 9] = current_y - g.oy - g.h;
            vertices[i * 24 + 10] = g.x;
            vertices[i * 24 + 11] = g.y + g.h;
            vertices[i * 24 + 12] = current_x + g.ox + g.w;
            vertices[i * 24 + 13] = current_y - g.oy;
            vertices[i * 24 + 14] = g.x + g.w;
            vertices[i * 24 + 15] = g.y;
            vertices[i * 24 + 16] = current_x + g.ox;
            vertices[i * 24 + 17] = current_y - g.oy - g.h;
            vertices[i * 24 + 18] = g.x;
            vertices[i * 24 + 19] = g.y + g.h;
            vertices[i * 24 + 20] = current_x + g.ox + g.w;
            vertices[i * 24 + 21] = current_y - g.oy - g.h;
            vertices[i * 24 + 22] = g.x + g.w;
            vertices[i * 24 + 23] = g.y + g.h;

            current_x += g.w_space;

            // Consider kernings
            const char next = str[i + 1];
            for (const auto& x : g.kernings) {
                if (x.first == next) {
                    current_x += x.second;
                    break;
                }
            }

        }

        for (int i = 0; i < len * 24; ) {
            i += 2;
            vertices[i++] /= font.texture_width;
            vertices[i++] /= font.texture_height;
        }

        // 1. bind Vertex Array Object
        unsigned int VAO_;
        unsigned int VBO_;
        glGenVertexArrays(1, &VAO_);
        glBindVertexArray(VAO_);

        // 2. copy our vertices array in a buffer for OpenGL to use
        glGenBuffers(1, &VBO_);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_);
        GLenum usage;
        if (dynamic) {
            usage = GL_DYNAMIC_DRAW;
        }
        else {
            usage = GL_STATIC_DRAW;
        }
        glBufferData(GL_ARRAY_BUFFER, len * 24 * sizeof(float), vertices.data(), usage);

        // 3. then set our vertex attributes pointers
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));
        glEnableVertexAttribArray(1);

        VAO = VAO_;
        VBO = VBO_;

        n_vertices = len * 6;
    }

    ~Writing() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void Update(const char* str) {

        if (!dynamic) {
            std::cerr << "Error in Writing::Update: cannot update static writing.\n";
            return;
        }

        int len = strlen(str);
        std::vector<float> vertices = GetStringVertices(str, font);        

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, len * 24 * sizeof(float), vertices.data());

        n_vertices = len * 6;
    }

    void Render(const Shader& shader, unsigned int texture, int panel_x, int panel_y) const {
        shader.use();

        glm::mat4 world(1.0f);
        world = glm::translate(world, glm::vec3(x + panel_x, y + panel_y, 0.f));
        shader.SetMat4("world", world);

        if (highlighted) {
            shader.SetVec3("color", highlighted_color);
        }
        else {
            shader.SetVec3("color", normal_color);
        }

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, n_vertices);
    }

    // Temporarily deleted for safety
    Writing(const Writing& other) = delete;
    Writing& operator=(const Writing& other) = delete;
    Writing& operator=(Writing&& other) = delete;

    Writing(Writing&& other) : 
        VAO(other.VAO), 
        VBO(other.VBO),
        x(other.x), 
        y(other.y), 
        n_vertices(other.n_vertices), 
        highlighted(other.highlighted),
        font(other.font),
        dynamic(other.dynamic)
    {
        // TODO check this trick
        other.VAO = -1;
        other.VBO = -1;
    }

};

const glm::vec3 Writing::normal_color = glm::vec3(1.f);
const glm::vec3 Writing::highlighted_color = glm::vec3(1.f, 1.f, 0.f);


struct RectBackground {
    unsigned int VAO;
    unsigned int VBO;

    RectBackground(int width, int height) {
        float vertices[] = {
            // xy-pos       // xy-tex
            0.f,          0.f - height,
            0.f,          0.f,
            0.f + width,  0.f - height,
            0.f,          0.f,
            0.f + width,  0.f,
            0.f + width,  0.f - height,
        };

        // 1. bind Vertex Array Object
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // 2. copy our vertices array in a buffer for OpenGL to use
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 3. then set our vertex attributes pointers
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void Render(const Shader& shader, int panel_x, int panel_y) const {
        shader.use();

        glm::mat4 world(1.0f);
        world = glm::translate(world, glm::vec3(panel_x, panel_y, 0.f));
        shader.SetMat4("world", world);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    ~RectBackground() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    // Temporarily deleted for safety
    RectBackground(const RectBackground& other) = delete;
    RectBackground& operator=(const RectBackground& other) = delete;
    RectBackground& operator=(RectBackground&& other) = delete;

    RectBackground(RectBackground&& other) :
        VAO(other.VAO),
        VBO(other.VBO)
    {
        // TODO check this trick
        other.VAO = -1;
        other.VBO = -1;
    }
};


struct Panel {

    std::vector<Writing> writings;
    int x;
    int y;
    std::optional<RectBackground> background;

    Panel(int x_, int y_) : x(x_), y(y_) {}

    Panel(int x_, int y_, int w, int h, bool has_background = true) : x(x_), y(y_) {
        if (has_background) {
            background.emplace(w, h);
        }
    }

    void Render(const Shader& shader, const Shader& shader_background, unsigned int texture) const {
        if (background.has_value()) {
            background.value().Render(shader_background, x, y);
        }
        for (const auto& writing : writings) {
            writing.Render(shader, texture, x, y);
        }
    }

    void AddWriting(const char* str, int x, int y, const Font& font, bool dynamic = false, bool highlighted = false) {
        writings.emplace_back(str, x, y, font, dynamic, highlighted);
    }

};

struct UI {

    Shader shader;
    Shader shader_background;
    Font font;
    std::map<std::string, std::pair<Panel, bool>> panel_map;

    UI() : shader("glyph"), shader_background("background") {
        if (!ReadFont((std::filesystem::path(kFontRoot) / std::filesystem::path("centaur_regular_32.xml")).string().c_str(), font)) {
            std::cerr << "UI::UI: can't read font!\n";
        }

        glm::mat4 projection = glm::ortho(0.f, (float)kWindowWidth, 0.f, (float)kWindowHeight, 0.1f, 2.f);
        shader.use();
        shader.SetMat4("projection", projection);
        shader_background.use();
        shader_background.SetMat4("projection", projection);

        Panel main_menu(1550, 800);
        main_menu.AddWriting("New game 1 player", 0, 0, font);
        main_menu.AddWriting("New game 2 players", 0, -font.h_space, font);
        main_menu.AddWriting("Quit", 0, -font.h_space * 2, font);
        AddPanel("main_menu", std::move(main_menu), true);

        Panel end_game(500, 800);
        end_game.AddWriting("Congratulations! You won!", 0, 0, font, false);
        end_game.AddWriting("Score: 0   ", 0, -font.h_space, font, true, false);
        end_game.AddWriting("Back to menu", 0, -font.h_space * 4, font, false, true);
        AddPanel("end_game", std::move(end_game), false);

        Panel game_over(500, 800);
        game_over.AddWriting("Game over", 0, 0, font, false);
        game_over.AddWriting("Score: 0   ", 0, -font.h_space, font, true, false);
        game_over.AddWriting("Back to menu", 0, -font.h_space * 4, font, false, true);
        AddPanel("game_over", std::move(game_over), false);

        Panel game_ui(500, 1075);
        game_ui.AddWriting("Lives: 3 ", 100, 0, font, true, false);
        game_ui.AddWriting("Score: 0   ", 650, 0, font, true, false);
        AddPanel("game_ui", std::move(game_ui), false);

        Panel pause(850, 600, 220, font.h_space*2);
        pause.AddWriting("Resume", 0, 0, font, false, true);
        pause.AddWriting("Back to menu", 0, -font.h_space, font, false);
        AddPanel("pause", std::move(pause), false);

        Panel transition(890, 600, 140, font.h_space);
        transition.AddWriting("Stage  1", 5, 0, font, true, false);
        AddPanel("transition", std::move(transition), false);

    }

    void Render() const {
        for (const auto& x : panel_map) {
            if (x.second.second) {
                x.second.first.Render(shader, shader_background, font.texture);
            }
        }
    }

    void AddPanel(std::string name, Panel panel, bool active = true) {
        panel_map.emplace(std::move(name), std::make_pair(std::move(panel), active));
    }

};


#endif // NIKMAN_UI_H