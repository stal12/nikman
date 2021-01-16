#if !defined NIKMAN_ENTITY_H
#define NIKMAN_ENTITY_H

#include <filesystem>
#include <vector>
#include <random>
#include <bitset>

#include <glad/glad.h>
#include <stb_image.h>

#include "shader.h"
#include "level.h"


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

struct Slot {
    unsigned char walls = 0;    // wasd 0123
    bool crust = true;

    Slot() {}
};


struct Map {

    int h = 5;
    int w = 5;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int remaining_crusts = h * w;

    std::vector<Slot> grid;

    Map(int h_, int w_, const LevelDesc& level) : shader("map"), h(h_), w(w_), grid(h * w) {

        MakeRect(1.f, 1.f, VAO, VBO);

        int width, height;
        texture = MakeTexture("map.png", width, height);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(w, h, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

        FillWalls(level.ver_walls, level.hor_walls);

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
                grid[pos.first - 1 + pos.second * w].walls |= (1 << 3);
            }
            if (pos.first < w) {
                grid[pos.first + pos.second * w].walls |= (1 << 1);
            }
        }

        for (const auto& pos : hor_walls) {
            if (pos.second > 0) {
                grid[pos.first + (pos.second - 1) * w].walls |= (1 << 0);
            }
            if (pos.second < h) {
                grid[pos.first + pos.second * w].walls |= (1 << 2);
            }
        }

    }

    void LoadLevel(const LevelDesc& level) {
        h = level.h;
        w = level.w;
        
        grid = std::vector<Slot>(h * w);
        for (const auto& x : level.pumpkin_home) {
            grid[x.first + x.second * w].crust = false;
        }
        for (const auto& x : level.hammers) {
            grid[x.first + x.second * w].crust = false;
        }
        grid[level.player_pos.first + level.player_pos.second * w].crust = false;

        FillWalls(level.ver_walls, level.hor_walls);
        remaining_crusts = h * w - level.hammers.size() - level.pumpkin_home.size() - 1;

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(w, h, 1.f));
        shader.SetMat4("world", world);
        shader.SetFloat("h", (float) h);
        shader.SetFloat("w", (float) w);
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
    float h = 5.f;
    float w = 5.f;

    Wall(float h_, float w_, LevelDesc level_) : shader("wall"), h(h_), w(w_) {

        MakeRect(0.2f, 1.2f, VAO, VBO);

        int width, height;
        texture = MakeTexture("wall.png", width, height);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

        ver_positions = level_.ver_walls;
        hor_positions = level_.hor_walls;

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
                -w / 2 + pos.first * size,
                -h / 2 + size / 2 + pos.second * size,
                0.f)
            );
            world = glm::scale(world, glm::vec3(size, size, 1.f));
            shader.SetMat4("world", world);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        for (const auto& pos : hor_positions) {
            world = glm::mat4(1.f);
            world = glm::translate(world, glm::vec3(
                -w / 2 + size / 2 + pos.first * size,
                -h / 2 + pos.second * size,
                0.f)
            );
            world = glm::scale(world, glm::vec3(size, size, 1.f));
            world = glm::rotate(world, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
            shader.SetMat4("world", world);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

    }

    void LoadLevel(const LevelDesc& level) {
        h = level.h;
        w = level.w;
        ver_positions = level.ver_walls;
        hor_positions = level.hor_walls;
    }

};



struct Player {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int h;
    int w;

    int x, y;
    int next_x, next_y;
    float precise_x;
    float precise_y;
    float t = 0;
    unsigned char direction;
    bool just_hit;
    float time_after_hit;
    int lives = 3;
    
    const float speed = 4.5f;    
    const float hit_recover_time = 1.0f;
    const float blink_freq = 50.0f;

    enum class State { Idle, Moving };
    State state = State::Idle;

    std::vector<Slot>& grid;


    Player(int h_, int w_, std::vector<Slot>& grid_) : shader("player"), h(h_), w(w_), grid(grid_) {

        MakeRect(0.7f, 0.7f, VAO, VBO);

        int width, height;
        texture = MakeTexture("player.png", width, height, true, true);

        x = 0;
        y = 0;
        next_x = 0;
        next_y = 0;

        shader.use();
        glm::mat4 world(1.f);
        world = glm::translate(world, glm::vec3(
            -w / 2.f + size / 2.f + size * (x * (1 - t) + next_x * t),
            -h / 2.f + size / 2.f + size * (y * (1 - t) + next_y * t),
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

        unsigned char walls = grid[y * w + x].walls;

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

    void Update(float delta, unsigned int wasd, unsigned int& eaten) {

        if (just_hit) {
            time_after_hit += delta;
            if (time_after_hit > hit_recover_time) {
                just_hit = false;                
            }
        }

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

        precise_x = (x * (1 - t) + next_x * t);
        precise_y = (y * (1 - t) + next_y * t);

        // Eat crusts
        float dist_threshold = 0.2f;
        eaten = 0;
        if (t < dist_threshold) {
            if (grid[y * w + x].crust) {
                ++eaten;
                grid[y * w + x].crust = false;
            }
        }
        if (t > 1 - dist_threshold) {
            if (grid[next_y * w + next_x].crust) {
                ++eaten;
                grid[next_y * w + next_x].crust = false;
            }
        }
    }

    void Render() const {

        if (!just_hit || sinf(time_after_hit * blink_freq) > -0.5) {
            shader.use();

            glm::mat4 world(1.f);
            world = glm::translate(world, glm::vec3(
                -w / 2.f + size / 2.f + size * precise_x,
                -h / 2.f + size / 2.f + size * precise_y,
                0.f)
            );
            world = glm::scale(world, glm::vec3(size, size, 1.f));
            shader.SetMat4("world", world);

            glBindTexture(GL_TEXTURE_2D, texture);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    void LoadLevel(const LevelDesc& level) {
        h = level.h;
        w = level.w;
        x = level.player_pos.first;
        y = level.player_pos.second;
        next_x = x;
        next_y = y;
        t = 0;
        state = State::Idle;
        just_hit = false;

        shader.use();
        glm::mat4 world(1.f);
        world = glm::translate(world, glm::vec3(
            -w / 2.f + size / 2.f + size * (x * (1 - t) + next_x * t),
            -h / 2.f + size / 2.f + size * (y * (1 - t) + next_y * t),
            0.f)
        );
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
    }

};


struct Crust {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int h;
    int w;

    std::vector<Slot>& grid;

    Crust(int h_, int w_, std::vector<Slot>& grid_) : shader("crust"), h(h_), w(w_), grid(grid_) {

        MakeRect(0.2f, 0.4f, VAO, VBO);

        int width, height;
        texture = MakeTexture("crust.png", width, height, true, true);

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

        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {

                if (grid[y * w + x].crust) {

                    glm::mat4 world(1.f);
                    world = glm::translate(world, glm::vec3(
                        -w / 2.f + size / 2.f + size * x,
                        -h / 2.f + size / 2.f + size * y,
                        0.f
                    ));

                    world = glm::scale(world, glm::vec3(size, size, 1.f));
                    shader.SetMat4("world", world);

                    glDrawArrays(GL_TRIANGLES, 0, 6);

                }


            }
        }

    }

    void LoadLevel(const LevelDesc& level) {

        h = level.h;
        w = level.w;

    }


};


struct RandomGuy {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int h;
    int w;

    int x, y;
    int next_x, next_y;
    float precise_x, precise_y;
    float t = 0;
    const float speed = 2.f;
    unsigned char direction;

    std::vector<Slot>& grid;

    std::random_device rd;
    std::mt19937 mt;


    RandomGuy(int h_, int w_, std::vector<Slot>& grid_) : shader("RandomGuy"), h(h_), w(w_), grid(grid_), mt(rd()) {

        MakeRect(0.4f, 0.4f, VAO, VBO);

        int width, height;
        texture = MakeTexture("RandomGuy.png", width, height, true, true);

        x = 4;
        y = 4;
        next_x = 3;
        next_y = 4;
        direction = 2;  // A

        shader.use();
        glm::mat4 world(1.f);
        world = glm::translate(world, glm::vec3(
            -w / 2.f + size / 2.f + size * (x * (1 - t) + next_x * t),
            -h / 2.f + size / 2.f + size * (y * (1 - t) + next_y * t),
            0.f)
        );
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

    }

    ~RandomGuy() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void SetNewDir() {

        const unsigned char walls = grid[y * w + x].walls;
        
        const unsigned char possible_dirs = ~walls & 15;
        
        const unsigned char backward_dir = (direction >> 2) | ((direction << 2) & 15);
        
        const unsigned char possible_dirs_without_back = possible_dirs & ~backward_dir;

        if (possible_dirs_without_back == 0) {
            direction = backward_dir;
        }
        else {
            const int n_dirs = std::bitset<8>(possible_dirs_without_back).count();
            if (n_dirs == 1) {
                direction = possible_dirs_without_back;
            }
            else {
                std::uniform_int_distribution dist(0, n_dirs - 1);
                int random_int = dist(mt);

                int i;
                for (i = 0; i < 4; ++i) {
                    if (!(possible_dirs_without_back & (1 << i))) {
                        continue;
                    }
                    if (random_int == 0) {
                        break;
                    }
                    random_int--;                    
                }

                direction = (1 << i);
            }
        }
    }

    void SetNextXY() {
        if (direction & 1) {
            next_x = x;
            next_y = y + 1;
        }
        else if (direction & 2) {
            next_x = x - 1;
            next_y = y;
        }
        else if (direction & 4) {
            next_x = x;
            next_y = y - 1;
        }
        else {
            next_x = x + 1;
            next_y = y;
        }
    }

    void Update(float delta, float player_x, float player_y, bool& hit_player) {

        t += delta * speed;
        if (t >= 1.f) {
            x = next_x;
            y = next_y;
            t -= 1.f;
            SetNewDir();
            SetNextXY();
        }

        precise_x = x * (1 - t) + next_x * t;
        precise_y = y * (1 - t) + next_y * t;

        // Check collision with player
        constexpr float threshold = 0.1;
        float squared_dist = (precise_x - player_x)* (precise_x - player_x) + (precise_y - player_y) * (precise_y - player_y);
        if (squared_dist < threshold) {
            hit_player = true;
        }

    }

    void Render() const {
        shader.use();

        glm::mat4 world(1.f);
        world = glm::translate(world, glm::vec3(
            -w / 2.f + size / 2.f + size * precise_x,
            -h / 2.f + size / 2.f + size * precise_y,
            0.f)
        );
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void LoadLevel(const LevelDesc& level) {
        h = level.h;
        w = level.w;
        x = level.pumpkin_home.front().first;   // TODO: implement enemy beginning
        y = level.pumpkin_home.front().second;
        next_x = x;
        next_y = y;
        t = 0;

        shader.use();
        glm::mat4 world(1.f);
        world = glm::translate(world, glm::vec3(
            -w / 2.f + size / 2.f + size * (x * (1 - t) + next_x * t),
            -h / 2.f + size / 2.f + size * (y * (1 - t) + next_y * t),
            0.f)
        );
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
    }

};



#endif