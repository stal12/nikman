#if !defined NIKMAN_ENTITY_H
#define NIKMAN_ENTITY_H

#include <filesystem>
#include <vector>

#include <glad/glad.h>
#include <stb_image.h>

#include "shader.h"



void MakeRect(float width, float height, unsigned int& VAO, unsigned int& VBO) {

    float vertices[] = {
        // xy-pos       // xy-tex
        -width / 2, +height / 2,   0.f, 1.f,
        -width / 2, -height / 2,   0.f, 0.f,
        +width / 2, +height / 2,   1.f, 1.f,
        -width / 2, -height / 2,   0.f, 0.f,
        +width / 2, -height / 2,   1.f, 0.f,
        +width / 2, +height / 2,   1.f, 1.f,
    };

    // 1. bind Vertex Array Object
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 2. copy our vertices array in a buffer for OpenGL to use
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3. then set our vertex attributes pointers
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));
    glEnableVertexAttribArray(1);
}


unsigned int MakeTexture(const char* filename, bool nearest = false, bool alpha = false) {
    // Texture 
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLint interp = nearest ? GL_NEAREST : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interp);

    // load and generate the texture
    int width, height, nrChannels;

    std::filesystem::path texture_path = std::filesystem::path(kTextureRoot) / std::filesystem::path(filename);
    std::string texture_path_string = texture_path.string();
    const char* texture_path_cstring = texture_path_string.c_str();

    unsigned char* data = stbi_load(texture_path_cstring, &width, &height, &nrChannels, 0);
    if (data)
    {
        //GLenum format = alpha ? GL_RGBA : GL_RGB;
        GLint internal_format = alpha ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}



struct Slot {
    unsigned char walls = 0;    // wasd 0123
    bool crust = true;

    Slot() {}
};


struct Map {

    const int size = 5;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int remaining_crusts = size * size;

    std::vector<Slot> grid;

    Map() : shader("map"), grid(size* size) {

        MakeRect(1.f, 1.f, VAO, VBO);

        texture = MakeTexture("map.png");

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

    }

    ~Map() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void Render() const {
        shader.use();
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void FillWalls(
        const std::vector<std::pair<int, int>>& ver_walls,
        const std::vector<std::pair<int, int>>& hor_walls) {

        for (const auto& pos : ver_walls) {
            if (pos.first > 0) {
                grid[pos.first - 1 + pos.second * size].walls |= (1 << 3);
            }
            if (pos.first < size) {
                grid[pos.first + pos.second * size].walls |= (1 << 1);
            }
        }

        for (const auto& pos : hor_walls) {
            if (pos.second > 0) {
                grid[pos.first + (pos.second - 1) * size].walls |= (1 << 0);
            }
            if (pos.second < size) {
                grid[pos.first + pos.second * size].walls |= (1 << 2);
            }
        }

    }

};


struct Wall {

    const float size = 1.f;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    std::vector<std::pair<int, int>> ver_positions;
    std::vector<std::pair<int, int>> hor_positions;
    float map_size = 5.f;  // this should change with the map

    Wall(float map_size_) : shader("wall"), map_size(map_size_) {

        MakeRect(0.2f, 1.2f, VAO, VBO);

        texture = MakeTexture("wall.png");

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

        ver_positions = { {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4},
                          {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4},
            {4, 1}, {4, 2} };
        hor_positions = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
                            {0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5},
                            {0, 1}, {1, 1}, {2, 1}, {3, 1},
        };

    }

    ~Wall() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void Render() const {
        shader.use();
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        // TODO: use instancing
        glm::mat4 world;
        for (const auto& pos : ver_positions) {
            world = glm::mat4(1.f);
            world = glm::translate(world, glm::vec3(
                -map_size / 2 + pos.first * size,
                -map_size / 2 + size / 2 + pos.second * size,
                0.f)
            );
            world = glm::scale(world, glm::vec3(size, size, 1.f));
            shader.SetMat4("world", world);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        for (const auto& pos : hor_positions) {
            world = glm::mat4(1.f);
            world = glm::translate(world, glm::vec3(
                -map_size / 2 + size / 2 + pos.first * size,
                -map_size / 2 + pos.second * size,
                0.f)
            );
            world = glm::scale(world, glm::vec3(size, size, 1.f));
            world = glm::rotate(world, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
            shader.SetMat4("world", world);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

    }

};



struct Player {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int map_size;

    int x, y;
    int next_x, next_y;
    float t = 0;
    const float speed = 1.5f;
    unsigned char direction;

    enum class State { Idle, Moving };
    State state = State::Idle;

    std::vector<Slot>& grid;


    Player(int map_size_, std::vector<Slot>& grid_) : shader("player"), map_size(map_size_), grid(grid_) {

        MakeRect(0.7f, 0.7f, VAO, VBO);

        texture = MakeTexture("player.png", true, true);

        x = 0;
        y = 0;
        next_x = 0;
        next_y = 0;

        shader.use();
        glm::mat4 world(1.f);
        world = glm::translate(world, glm::vec3(
            -map_size / 2.f + size / 2.f + size * (x * (1 - t) + next_x * t),
            -map_size / 2.f + size / 2.f + size * (y * (1 - t) + next_y * t),
            0.f)
        );
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

    }

    ~Player() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void FindNext(unsigned char wasd) {

        unsigned char walls = grid[y * map_size + x].walls;

        if ((wasd & 1) && !(walls & 1)) {
            next_x = x;
            next_y = y + 1;
            direction = 1;
            state = State::Moving;
        }
        else if ((wasd & 2) && !(walls & 2)) {
            next_x = x - 1;
            next_y = y;
            direction = 2;
            state = State::Moving;
        }
        else if ((wasd & 4) && !(walls & 4)) {
            next_x = x;
            next_y = y - 1;
            direction = 4;
            state = State::Moving;
        }
        else if ((wasd & 8) && !(walls & 8)) {
            next_x = x + 1;
            next_y = y;
            direction = 8;
            state = State::Moving;
        }
        else {
            state = State::Idle;
        }
    }

    void Update(float delta, unsigned int wasd) {
        if (state == State::Moving) {
            t += delta * speed;
            if (((direction >> 2) | ((direction << 2) & 15)) & wasd) {
                direction = ((direction >> 2) | ((direction << 2) & 15));
                std::swap(x, next_x);
                std::swap(y, next_y);
                t = 1 - t;
            }
            if (t >= 1.f) {
                x = next_x;
                y = next_y;
                t -= 1.f;
                FindNext(wasd);
            }
        }
        else {
            FindNext(wasd);
            t = 0;
        }

        // Eat crusts
        float dist_threshold = 0.2f;
        if (t < dist_threshold) {
            grid[y * map_size + x].crust = false;
        }
        if (t > 1 - dist_threshold) {
            grid[next_y * map_size + next_x].crust = false;
        }
    }

    void Render() const {
        shader.use();

        glm::mat4 world(1.f);
        world = glm::translate(world, glm::vec3(
            -map_size / 2.f + size / 2.f + size * (x * (1 - t) + next_x * t),
            -map_size / 2.f + size / 2.f + size * (y * (1 - t) + next_y * t),
            0.f)
        );
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

};


struct Crust {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int map_size;

    std::vector<Slot>& grid;

    Crust(int map_size_, std::vector<Slot>& grid_) : shader("crust"), map_size(map_size_), grid(grid_) {

        MakeRect(0.2f, 0.4f, VAO, VBO);

        texture = MakeTexture("crust.png", true, true);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

    }

    ~Crust() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }


    void Render() const {

        shader.use();

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        for (int x = 0; x < map_size; ++x) {
            for (int y = 0; y < map_size; ++y) {

                if (grid[y * map_size + x].crust) {

                    glm::mat4 world(1.f);
                    world = glm::translate(world, glm::vec3(
                        -map_size / 2.f + size / 2.f + size * x,
                        -map_size / 2.f + size / 2.f + size * y,
                        0.f
                    ));

                    world = glm::scale(world, glm::vec3(size, size, 1.f));
                    shader.SetMat4("world", world);

                    glDrawArrays(GL_TRIANGLES, 0, 6);

                }


            }
        }

    }

};



#endif