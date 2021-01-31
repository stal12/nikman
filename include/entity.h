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
    // Default data is just a crust
    unsigned char data = 16;    // wasd 0123 crust 4 weapon 5 home 6 teleport 7

    bool Crust() const {
        return data & 16;
    }

    bool Weapon() const {
        return data & 32;
    }

    bool Home() const {
        return data & 64;
    }

    bool Teleport() const {
        return data & 128;
    }

    void SetCrust() {
        data |= 16;
    }    

    void RemoveCrust() {
        data &= ~16;
    }

    void SetWeapon() {
        data |= 32;
    }

    void RemoveWeapon() {
        data &= ~32;
    }

    void SetHome() {
        data |= 64;
    }

    void RemoveHome() {
        data &= ~64;
    }

    void SetTeleport() {
        data |= 128;
    }

    void RemoveTeleport() {
        data &= ~128;
    }

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

    Map(int h_, int w_, const LevelDesc& level) : shader("map"), h(h_), w(w_), grid(h* w) {

        MakeRect(1.f, 1.f, VAO, VBO);

        int width, height;
        texture = MakeTexture("map.png", width, height, false);

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
                grid[pos.first - 1 + pos.second * w].data |= (1 << 3);
            }
            if (pos.first < w) {
                grid[pos.first + pos.second * w].data |= (1 << 1);
            }
        }

        for (const auto& pos : hor_walls) {
            if (pos.second > 0) {
                grid[pos.first + (pos.second - 1) * w].data |= (1 << 0);
            }
            if (pos.second < h) {
                grid[pos.first + pos.second * w].data |= (1 << 2);
            }
        }

    }

    void LoadLevel(const LevelDesc& level) {
        h = level.h;
        w = level.w;

        grid = std::vector<Slot>(h * w);
        for (const auto& x : level.pumpkin_home) {
            grid[x.first + x.second * w].RemoveCrust();
            grid[x.first + x.second * w].SetHome();
        }
        for (const auto& x : level.weapons) {
            grid[x.first + x.second * w].RemoveCrust();
            grid[x.first + x.second * w].SetWeapon();
        }
        for (const auto& x : level.mud) {
            grid[x.first + x.second * w].RemoveCrust();
        }
        for (const auto& x : level.empty) {
            grid[x.first + x.second * w].RemoveCrust();
        }
        for (const auto& x : level.teleports) {
            grid[x.first + x.second * w].RemoveCrust();
            grid[x.first + x.second * w].SetTeleport();
        }
        grid[level.player_pos.first + level.player_pos.second * w].RemoveCrust();

        FillWalls(level.ver_walls, level.hor_walls);
        remaining_crusts = h * w - 1 -
            level.weapons.size() - 
            level.pumpkin_home.size() - 
            level.mud.size() - 
            level.teleports.size() -
            level.empty.size();

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(w, h, 1.f));
        shader.SetMat4("world", world);
        shader.SetFloat("h", (float)h);
        shader.SetFloat("w", (float)w);
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
        texture = MakeTexture("wall.png", width, height, true);

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


struct Teleport {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int h;
    int w;
    std::vector<std::pair<int, int>> teleports;

    std::vector<Slot>& grid;
    std::random_device rd;
    std::mt19937 mt;


    Teleport(int h_, int w_, std::vector<Slot>& grid_) : shader("teleport"), h(h_), w(w_), grid(grid_), mt(rd()) {

        MakeRect(0.7f, 0.7f, VAO, VBO);

        int width, height;
        texture = MakeTexture("teleport.png", width, height, false, true);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

    }

    ~Teleport() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }


    void Render() const {

        shader.use();

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        for (const auto& x : teleports) {
            glm::mat4 world(1.f);
            world = glm::translate(world, glm::vec3(
                -w / 2.f + size / 2.f + size * x.first,
                -h / 2.f + size / 2.f + size * x.second,
                0.f
            ));

            world = glm::scale(world, glm::vec3(size, size, 1.f));
            shader.SetMat4("world", world);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    void LoadLevel(const LevelDesc& level) {

        h = level.h;
        w = level.w;
        teleports = std::move(level.teleports);

    }

    void RandomDestination(int& x, int& y) {
        std::uniform_int_distribution dist(0, static_cast<int>(teleports.size() - 1));
        while (true) {
            int random_index = dist(mt);
            const auto& pos = teleports[random_index];
            if (pos.first != x || pos.second != y) {
                x = pos.first;
                y = pos.second;
                break;
            }
        }
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
    bool just_teleported;
    float time_after_hit;
    int lives = 3;

    const float speed = 4.5f;
    const float hit_recover_time = 1.0f;
    const float blink_freq = 50.0f;

    enum class State { Idle, Moving };
    State state = State::Idle;

    std::vector<Slot>& grid;
    Teleport& teleport;


    Player(int h_, int w_, std::vector<Slot>& grid_, Teleport& teleport_) : shader("player"), h(h_), w(w_), grid(grid_), teleport(teleport_) {

        MakeRect(0.7f, 0.7f, VAO, VBO);

        int width, height;
        texture = MakeTexture("player.png", width, height, false, true);

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

        unsigned char walls = grid[y * w + x].data & 15;

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

    void Update(float delta, unsigned int wasd, unsigned int& eaten, bool& weapon) {

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
                if (grid[y * w + x].Teleport() && !just_teleported) {
                    teleport.RandomDestination(x, y);
                    next_x = x;
                    next_y = y;
                    just_teleported = true;
                }
                else {
                    just_teleported = false;
                }
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

        // Eat crusts, pick up weapons
        float dist_threshold = 0.2f;
        eaten = 0;
        if (t < dist_threshold) {
            if (grid[y * w + x].Crust()) {
                ++eaten;
                grid[y * w + x].RemoveCrust();
            }
            if (grid[y * w + x].Weapon()) {
                weapon = true;
                grid[y * w + x].RemoveWeapon();
            }
        }
        if (t > 1 - dist_threshold) {
            if (grid[next_y * w + next_x].Crust()) {
                ++eaten;
                grid[next_y * w + next_x].RemoveCrust();
            }
            if (grid[next_y * w + next_x].Weapon()) {
                weapon = true;
                grid[next_y * w + next_x].RemoveWeapon();
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
        precise_x = x;
        precise_y = y;
        next_x = x;
        next_y = y;
        t = 0;
        state = State::Idle;
        just_hit = false;
        just_teleported = false;

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
        texture = MakeTexture("crust.png", width, height, false, true);

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

                if (grid[y * w + x].Crust()) {

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

struct Weapon {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int h;
    int w;
    bool player_armed = false;
    float player_x;
    float player_y;
    float t;
    const float duration = 3.f;
    const float blink_freq = 50.0f;

    std::vector<Slot>& grid;

    Weapon(int h_, int w_, std::vector<Slot>& grid_) : shader("sword"), h(h_), w(w_), grid(grid_) {

        MakeRect((0.6f * 25) / 36, 0.6f, VAO, VBO);

        int width, height;
        texture = MakeTexture("sword.png", width, height, false, true);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

    }

    ~Weapon() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void Update(float delta, float player_x_, float player_y_) {
        t += delta;
        if (t > duration) {
            player_armed = false;
        }
        else {
            player_x = player_x_;
            player_y = player_y_;
        }
    }

    void SetPlayerArmed(bool player_armed_ = true) {
        player_armed = player_armed_;
        t = 0;
    }

    void Render() const {

        shader.use();

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {

                if (grid[y * w + x].Weapon()) {

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


        if (player_armed && (t < duration - 2.f || sinf(t * blink_freq) > -0.2)) {

            glm::mat4 world(1.f);
            world = glm::translate(world, glm::vec3(
                -w / 2.f + size / 2.f + size * player_x + size / 4.f,
                -h / 2.f + size / 2.f + size * player_y - size / 8.f,
                0.f
            ));

            world = glm::scale(world, glm::vec3(size / 1.5f, size / 1.5f, 1.f));
            shader.SetMat4("world", world);

            glDrawArrays(GL_TRIANGLES, 0, 6);

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
        texture = MakeTexture("RandomGuy.png", width, height, false, true);

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

        const unsigned char walls = grid[y * w + x].data & 15;

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
        float squared_dist = (precise_x - player_x) * (precise_x - player_x) + (precise_y - player_y) * (precise_y - player_y);
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
        precise_x = x;
        precise_y = y;
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

struct Ghost {

    enum class Color { Red, Yellow, Blue, Pink };
    enum class State { Chase, Scatter, Frightened, Home };

    static const char* const texture_array[4];
    static const float speed_array[4];

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
    int home_x, home_y;
    float t = 0;
    const float speed;
    unsigned char direction;
    Color color;
    State state;
    int target_x;
    int target_y;
    int scatter_x;
    int scatter_y;
    float state_t;
    float scatter_duration = 0.f;
    float chase_duration = 10.f;
    float frightened_duration = 5.f;
    float home_duration = 4.f;
    bool just_teleported;

    std::vector<Slot>& grid;
    Teleport& teleport;

    std::random_device rd;
    std::mt19937 mt;


    Ghost(Color color_, int h_, int w_, std::vector<Slot>& grid_, Teleport& teleport_) :
        shader("ghost"),
        color(color_),
        state(State::Home),
        speed(speed_array[static_cast<int>(color_)] * 0.5f),
        h(h_),
        w(w_),
        grid(grid_),
        mt(rd()),
        teleport(teleport_)
    {

        MakeRect(0.49f, 0.49f, VAO, VBO);

        int width, height;
        texture = MakeTexture(texture_array[static_cast<int>(color)], width, height, false, true);

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

    ~Ghost() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void DirToNext(unsigned char direction, int& next_x_ref, int& next_y_ref) {
        if (direction & 1) {
            next_x_ref = x;
            next_y_ref = y + 1;
        }
        else if (direction & 2) {
            next_x_ref = x - 1;
            next_y_ref = y;
        }
        else if (direction & 4) {
            next_x_ref = x;
            next_y_ref = y - 1;
        }
        else {
            next_x_ref = x + 1;
            next_y_ref = y;
        }
    }

    // Update current direction based on target tile
    void TargetTileDirection(unsigned char possible_dirs) {

        float min_dist = std::numeric_limits<float>::max();
        unsigned char min_direction = 16;

        for (unsigned char i = 1; i < 16; i <<= 1) {
            if (possible_dirs & i) {
                int next_x, next_y;
                DirToNext(i, next_x, next_y);
                const float dist = (next_x - target_x) * (next_x - target_x) + (next_y - target_y) * (next_y - target_y);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_direction = i;
                }
            }
        }

        direction = min_direction;

    }

    void SetNewDir(float player_x, float player_y, unsigned char player_dir, float red_x, float red_y) {

        const unsigned char walls = grid[y * w + x].data & 15;

        unsigned char possible_dirs = ~walls & 15;

        const unsigned char backward_dir = (direction >> 2) | ((direction << 2) & 15);

        if (state == State::Home) {
            // Remove directions outside of home
            if (possible_dirs & 1) {
                const int next_x = x;
                const int next_y = y + 1;
                if (!grid[next_y * w + next_x].Home())
                    possible_dirs &= ~1;
            }
            if (possible_dirs & 2) {
                const int next_x = x - 1;
                const int next_y = y;
                if (!grid[next_y * w + next_x].Home())
                    possible_dirs &= ~2;
            }
            if (possible_dirs & 4) {
                const int next_x = x;
                const int next_y = y - 1;
                if (!grid[next_y * w + next_x].Home())
                    possible_dirs &= ~4;
            }
            if (possible_dirs & 8) {
                const int next_x = x + 1;
                const int next_y = y;
                if (!grid[next_y * w + next_x].Home())
                    possible_dirs &= ~8;
            }
        }

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
                if (state == State::Frightened || state == State::Home) {
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
                else if (state == State::Scatter) {
                    // Approach scatter tile
                    TargetTileDirection(possible_dirs_without_back);
                }
                else if (state == State::Chase) {
                    // Determine target tile and approach it

                    // Red ghost example
                    if (color == Color::Red) {
                        target_x = player_x;
                        target_y = player_y;
                    }
                    else if (color == Color::Pink) {
                        if (player_dir & 1) {
                            target_x = player_x;
                            target_y = player_y + 4;
                        }
                        else if (player_dir & 2) {
                            target_x = player_x - 4;
                            target_y = player_y;
                        }
                        else if (player_dir & 4) {
                            target_x = player_x;
                            target_y = player_y - 4;
                        }
                        else {
                            target_x = player_x + 4;
                            target_y = player_y;
                        }
                    }
                    else if (color == Color::Blue) {
                        int player_front_two_x, player_front_two_y;
                        if (player_dir & 1) {
                            player_front_two_x = player_x;
                            player_front_two_y = player_y + 2;
                        }
                        else if (player_dir & 2) {
                            player_front_two_x = player_x - 2;
                            player_front_two_y = player_y;
                        }
                        else if (player_dir & 4) {
                            player_front_two_x = player_x;
                            player_front_two_y = player_y - 2;
                        }
                        else {
                            player_front_two_x = player_x + 2;
                            player_front_two_y = player_y;
                        }
                        // Double the vector from red to the position two tiles in front of the player
                        target_x = 2 * player_front_two_x - red_x;
                        target_y = 2 * player_front_two_y - red_y;
                    }
                    else if (color == Color::Yellow) {
                        const float dist = (precise_x - player_x) * (precise_x - player_x) + (precise_y - player_y) * (precise_y - player_y);
                        if (dist > 6.f) { // TODO remove magic number
                            target_x = player_x;
                            target_y = player_y;
                        }
                        else {
                            target_x = scatter_x;
                            target_y = scatter_y;
                        }
                    }

                    TargetTileDirection(possible_dirs_without_back);
                }
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

    void Update(float delta, float player_x, float player_y, unsigned char player_dir, float red_x, float red_y, bool& hit_player) {

        state_t += delta;
        if (state == State::Chase) {
            if (state_t >= chase_duration) {
                state = State::Scatter;
                target_x = scatter_x;
                target_y = scatter_y;
                state_t = 0;
            }
        }
        else if (state == State::Scatter) {
            if (state_t >= scatter_duration) {
                state = State::Chase;
                state_t = 0;
            }
        }
        else if (state == State::Frightened) {
            if (state_t >= frightened_duration) {
                state = State::Chase;
                state_t = 0;
            }
        }
        else if (state == State::Home) {
            if (state_t >= home_duration) {
                state = State::Chase;
                state_t = 0;
            }
        }

        t += delta * speed;
        if (t >= 1.f) {
            x = next_x;
            y = next_y;
            if (grid[y * w + x].Teleport() && !just_teleported) {
                teleport.RandomDestination(x, y);
                next_x = x;
                next_y = y;
                just_teleported = true;
            }
            else {
                just_teleported = false;
            }
            t -= 1.f;
            SetNewDir(player_x, player_y, player_dir, red_x, red_y);
            SetNextXY();
        }

        precise_x = x * (1 - t) + next_x * t;
        precise_y = y * (1 - t) + next_y * t;

        // Check collision with player
        constexpr float threshold = 0.1;
        float squared_dist = (precise_x - player_x) * (precise_x - player_x) + (precise_y - player_y) * (precise_y - player_y);
        if (squared_dist < threshold) {
            hit_player = true;
        }

    }

    void Killed() {
        x = home_x;
        y = home_y;
        next_x = x;
        next_y = y;
        state = State::Home;
        state_t = 0;
    }

    void Frighten() {
        state = State::Frightened;
        state_t = 0;
        std::swap(x, next_x);
        std::swap(y, next_y);
        direction = ((direction >> 2) | ((direction << 2) & 15));
        t = 1 - t;
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
        home_x = level.pumpkin_home.front().first;
        home_y = level.pumpkin_home.front().second;
        x = home_x;   // TODO: implement enemy beginning
        y = home_y;
        next_x = x;
        next_y = y;
        precise_x = x;
        precise_y = y;
        t = 0;
        state = State::Home;
        t = 0;
        just_teleported = false;
        if (color == Color::Red) {
            scatter_x = -1;
            scatter_y = -1;
        }
        else if (color == Color::Yellow) {
            scatter_x = w;
            scatter_y = -1;
        }
        else if (color == Color::Blue) {
            scatter_x = -1;
            scatter_y = h;
        }
        else if (color == Color::Pink) {
            scatter_x = w;
            scatter_y = h;
        }
        target_x = scatter_x;
        target_y = scatter_y;
        state_t = 0;

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

const char* const Ghost::texture_array[4] = { "ghost_red.png", "ghost_yellow.png" , "ghost_blue.png" , "ghost_pink.png" };
const float Ghost::speed_array[4] = { 2.f, 1.9f, 1.8f, 1.7f };

#endif