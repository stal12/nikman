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

#if !defined NIKMAN_ENTITY_H
#define NIKMAN_ENTITY_H

#include <filesystem>
#include <vector>
#include <random>
#include <bitset>

#include <glad/glad.h>
#include <stb_image.h>
#include <SFML/Audio.hpp>

#include "shader.h"
#include "level.h"

struct Point {
    float x;
    float y;
};

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

// Point a and Point b are the South-West and North-East corners in the texture atlas
void MakeRectWithCoords(float width, float height, Point a, Point b, unsigned int& VAO, unsigned int& VBO) {

    float vertices[] = {
        // xy-pos                  // xy-tex
        -width / 2, +height / 2,   a.x, b.y,
        -width / 2, -height / 2,   a.x, a.y,
        +width / 2, +height / 2,   b.x, b.y,
        -width / 2, -height / 2,   a.x, a.y,
        +width / 2, -height / 2,   b.x, a.y,
        +width / 2, +height / 2,   b.x, b.y,
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
    unsigned short data = 16;    // wasd walls 0 1 2 3 crust 4 weapon 5 home 6 teleport 7 dir 8 9 10 11
    float angle = 1.5f;

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

    unsigned char Direction() const {
        return (data & (256 + 512 + 1024 + 2048)) >> 8;
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

    // dir is a 4-bit value: w a s d
    void SetDirection(unsigned char dir) {
        data = (data & ~(512 + 256 + 1024 + 2048)) | (dir << 8);
    }

    Slot() {}
};

struct Sfondo {

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int remaining_crusts;

    std::vector<Slot> grid;

    Sfondo() : shader("sfondo") {

        MakeRect(2.f / 1.26f, 2.f, VAO, VBO);

        int width, height;
        texture = MakeTexture("cover.png", width, height, false, false);

        glm::mat4 world(1.f);
        world = glm::translate(world, glm::vec3(-1.f + 1.f / 1.26f, 0.f, 0.f));
        shader.use();
        shader.SetMat4("world", world);

    }

    ~Sfondo() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
        glDeleteTextures(1, &texture);
    }

    void Render() const {
        shader.use();
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    Sfondo(const Sfondo& other) = delete;
    Sfondo(Sfondo&& other) = delete;
    Sfondo& operator=(const Sfondo& other) = delete;
    Sfondo& operator=(Sfondo&& other) = delete;

};


struct Map {

    int h;
    int w;

    unsigned int VBO;
    unsigned int VAO;
    //unsigned int texture;
    Shader shader;
    int remaining_crusts;

    std::vector<Slot> grid;

    Map() : shader("map") {

        MakeRect(1.f, 1.f, VAO, VBO);

        int width, height;
        //texture = MakeTexture("map.png", width, height, false);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(w, h, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

        shader.SetFloat("x_s", 311.f / 384.f);
        shader.SetFloat("x_e", 383.f / 384.f);
        shader.SetFloat("y_s", 296.f / 369.f);
        shader.SetFloat("y_e", 368.f / 369.f);

    }

    ~Map() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void Render() const {
        shader.use();
        glBindTexture(GL_TEXTURE_2D, atlas);
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

    void LoadLevel(const LevelDesc& level, std::mt19937& mt) {
        h = level.h;
        w = level.w;

        std::uniform_real_distribution<float> dis(0.0f, 2.f * 3.14159f);

        grid = std::vector<Slot>(h * w);

        for (int i = 0; i < h * w; ++i) {
            grid[i].angle = dis(mt);
        }

        for (const auto& x : level.home) {
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
        grid[level.nik_pos.first + level.nik_pos.second * w].RemoveCrust();
        grid[level.ste_pos.first + level.ste_pos.second * w].RemoveCrust();

        FillWalls(level.ver_walls, level.hor_walls);
        remaining_crusts = h * w - 2 -
            level.weapons.size() -
            level.home.size() -
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

    Map(const Map& other) = delete;
    Map(Map&& other) = delete;
    Map& operator=(const Map& other) = delete;
    Map& operator=(Map&& other) = delete;

};

struct Wall {

    const float size = 1.f;

    unsigned int VBO;
    unsigned int VAO;
    //unsigned int texture;
    Shader shader;
    std::vector<std::pair<int, int>> ver_positions;
    std::vector<std::pair<int, int>> hor_positions;
    float h;
    float w;

    Wall() : shader("wall") {

        MakeRectWithCoords(14.f / 72.f, 86.f / 72.f, { 265.f / 384.f, 169.f / 369.f }, { 279.f / 384.f, 255.f / 369.f }, VAO, VBO);
        //MakeRect(14.f / 72.f, 86.f / 72.f, VAO, VBO);

        int width, height;
        //texture = MakeTexture("wall.png", width, height, true);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

    }

    ~Wall() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void Render() const {
        shader.use();
        glBindTexture(GL_TEXTURE_2D, atlas);
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

    Wall(const Wall& other) = delete;
    Wall(Wall&& other) = delete;
    Wall& operator=(const Wall& other) = delete;
    Wall& operator=(Wall&& other) = delete;

};

struct Teleport {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    //unsigned int texture;
    Shader shader;
    int h;
    int w;
    std::vector<std::pair<int, int>> teleports;

    std::vector<Slot>& grid;
    std::mt19937& mt;

    sf::SoundBuffer soundBuffer;
    sf::Sound sound;


    Teleport(std::vector<Slot>& grid_, std::mt19937& mt_) : shader("teleport"), grid(grid_), mt(mt_) {

        MakeRectWithCoords(55.f / 72.f, 55.f / 72.f, { 255.f / 384.f, 313.f / 369.f }, { 310.f / 384.f, 368.f / 369.f }, VAO, VBO);

        //int width, height;
        //texture = MakeTexture("teleport.png", width, height, false, true);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

        if (!soundBuffer.loadFromFile(SoundPath("waw.wav"))) {
            std::cerr << "Teleport::Teleport: can't open file \"waw.wav\"\n";
        }
        sound.setBuffer(soundBuffer);
        sound.setVolume(25.f);

    }

    ~Teleport() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }


    void Render() const {

        shader.use();

        glBindTexture(GL_TEXTURE_2D, atlas);
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
        sound.play();
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

    Teleport(const Teleport& other) = delete;
    Teleport(Teleport&& other) = delete;
    Teleport& operator=(const Teleport& other) = delete;
    Teleport& operator=(Teleport&& other) = delete;

};

struct Player {

    enum class Name { Nik, Ste };
    //static const char* const texture_array[2];

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    //unsigned int texture;
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
    static int lives;

    const float speed = 4.5f;
    const float hit_recover_time = 2.0f;
    const float blink_freq = 50.0f;

    enum class State { Idle, Moving };
    State state = State::Idle;

    std::vector<Slot>& grid;
    Teleport& teleport;

    Name name;
    bool armed = false;
    float weapon_t;
    const float baseWeaponDuration = 8.f;
    float weaponDuration;
    const float weaponVanishingTime = 2.f;
    bool weaponVanishing;

    sf::SoundBuffer liscioBuffer;
    sf::Sound liscio;

    sf::SoundBuffer gnamBuffer;
    sf::Sound gnam;

    Player(Name name_, std::vector<Slot>& grid_, Teleport& teleport_) :
        name(name_),
        shader("player"),
        grid(grid_),
        teleport(teleport_)
    {

        Point a, b;
        if (name == Name::Nik) {
            a = { 0.f, 57.f / 368.f };
            b = { 56.f / 384.f, 113.f / 369.f };
        }
        else {
            a = { 0.f, 0.f };
            b = { 56.f / 384.f, 56.f / 369.f };
        }

        MakeRectWithCoords(56.f / 72.f, 56.f / 72.f, a, b, VAO, VBO);

        //int width, height;
        //texture = MakeTexture(texture_array[static_cast<int>(name)], width, height, false, true);

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

        if (!liscioBuffer.loadFromFile(SoundPath("liscio.wav"))) {
            std::cerr << "Player::Player: can't open file \"liscio.wav\"\n";
        }
        liscio.setBuffer(liscioBuffer);
        liscio.setVolume(20);

        if (name == Name::Nik) {
            if (!gnamBuffer.loadFromFile(SoundPath("gnam_ste.wav"))) {
                std::cerr << "Player::Player: can't open file \"gnam_ste.wav\"\n";
            }
        }
        else {
            if (!gnamBuffer.loadFromFile(SoundPath("gnam_ste.wav"))) {
                std::cerr << "Player::Player: can't open file \"gnam_ste.wav\"\n";
            }
        }
        gnam.setBuffer(gnamBuffer);
        gnam.setVolume(1.f);

    }

    ~Player() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void FindNext(unsigned char wasd) {

        unsigned char walls = grid[y * w + x].data & 15;

        if ((wasd & 1) && !(walls & 1) && !grid[(y + 1) * w + x].Home()) {
            next_x = x;
            next_y = y + 1;
            direction = 1;
            state = State::Moving;
        }
        else if ((wasd & 2) && !(walls & 2) && !grid[y * w + (x - 1)].Home()) {
            next_x = x - 1;
            next_y = y;
            direction = 2;
            state = State::Moving;
        }
        else if ((wasd & 4) && !(walls & 4) && !grid[(y - 1) * w + x].Home()) {
            next_x = x;
            next_y = y - 1;
            direction = 4;
            state = State::Moving;
        }
        else if ((wasd & 8) && !(walls & 8) && !grid[y * w + (x + 1)].Home()) {
            next_x = x + 1;
            next_y = y;
            direction = 8;
            state = State::Moving;
        }
        else {
            state = State::Idle;
        }
    }

    void GrabWeapon() {
        weapon_t = 0;
        weaponVanishing = false;
        armed = true;
    }

    void Update(float delta, unsigned int wasd, float other_x, float other_y, unsigned int& eaten, bool& weapon) {

        // Update weapon
        if (armed) {
            weapon_t += delta;
            if (weapon_t > weaponDuration) {
                armed = false;
                liscio.play();
            }
            else if (!weaponVanishing && (weaponDuration - weapon_t < weaponVanishingTime)) {
                weaponVanishing = true;
            }
        }

        if (just_hit) {
            time_after_hit += delta;
            if (time_after_hit > hit_recover_time) {
                just_hit = false;
            }
        }

        if (state == State::Moving) {
            if (((direction >> 2) | ((direction << 2) & 15)) & wasd) {
                direction = ((direction >> 2) | ((direction << 2) & 15));
                std::swap(x, next_x);
                std::swap(y, next_y);
                t = 1.f - t;
                grid[y * w + x].SetDirection(direction);
            }

            constexpr float otherDistMin = 0.5f;
            if (isSte &&
                (precise_x - other_x) * (precise_x - other_x) + (precise_y - other_y) * (precise_y - other_y) < otherDistMin &&
                (direction == 1 && other_y > precise_y ||
                    direction == 2 && other_x < precise_x ||
                    direction == 4 && other_y < precise_y ||
                    direction == 8 && other_x > precise_x)
                ) {
            }
            else {
                t += delta * speed;
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
                grid[y * w + x].SetDirection(direction);
            }
        }
        else {
            t = 0.f;
            FindNext(wasd);
            grid[y * w + x].SetDirection(direction);
        }

        precise_x = (x * (1 - t) + next_x * t);
        precise_y = (y * (1 - t) + next_y * t);

        // Eat crusts, pick up weapons
        float dist_threshold = 0.2f;
        if (t < dist_threshold) {
            if (grid[y * w + x].Crust()) {
                ++eaten;
                gnam.play();
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
                gnam.play();
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

            float shiftX = 0;
            if (state == State::Moving) {
                shiftX = ((DirTo2Bit(direction) + 1) * 57.f) / 384.f;
            }
            shader.SetFloat("shiftX", shiftX);

            glBindTexture(GL_TEXTURE_2D, atlas);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    void LoadLevel(const LevelDesc& level, int current_level) {
        h = level.h;
        w = level.w;
        if (name == Name::Nik) {
            x = level.nik_pos.first;
            y = level.nik_pos.second;
        }
        else {
            x = level.ste_pos.first;
            y = level.ste_pos.second;
        }
        precise_x = x;
        precise_y = y;
        next_x = x;
        next_y = y;
        t = 0;
        state = State::Idle;
        just_hit = false;
        just_teleported = false;
        armed = false;
        weaponDuration = baseWeaponDuration - baseWeaponDuration * current_level / 40.f;

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

    Player(const Player& other) = delete;
    Player(Player&& other) = delete;
    Player& operator=(const Player& other) = delete;
    Player& operator=(Player&& other) = delete;

};

struct Crust {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    //unsigned int texture;
    Shader shader;
    int h;
    int w;

    std::vector<Slot>& grid;

    Crust(std::vector<Slot>& grid_) : shader("crust"), grid(grid_) {

        MakeRectWithCoords(16.f / 72.f, 32.f / 72.f, { 260.f / 384.f, 278.f / 369.f }, { 276.f / 384.f, 310.f / 369.f }, VAO, VBO);

        //int width, height;
        //texture = MakeTexture("crust.png", width, height, false, true);

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

        glBindTexture(GL_TEXTURE_2D, atlas);
        glBindVertexArray(VAO);

        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {

                if (grid[y * w + x].Crust()) {

                    const float angle = grid[y * w + x].angle;

                    glm::mat4 world(1.f);
                    world = glm::translate(world, glm::vec3(
                        -w / 2.f + size / 2.f + size * x,
                        -h / 2.f + size / 2.f + size * y,
                        0.f
                    ));

                    world = glm::scale(world, glm::vec3(size, size, 1.f));
                    world = glm::rotate(world, angle, glm::vec3(0.f, 0.f, 1.f));
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

    Crust(const Crust& other) = delete;
    Crust(Crust&& other) = delete;
    Crust& operator=(const Crust& other) = delete;
    Crust& operator=(Crust&& other) = delete;

};

struct Tile {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    unsigned int texture;
    Shader shader;
    int h;
    int w;

    std::vector<std::pair<int, int>> pos;

    Tile(const char* name) : shader("tile") {

        MakeRect(1.f, 1.f, VAO, VBO);

        int width, height;
        texture = MakeTexture((std::string(name) + ".png").c_str(), width, height, false, true);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

    }

    ~Tile() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }


    void Render() const {

        shader.use();

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);

        for (const auto& x : pos) {
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

    void LoadLevel(const LevelDesc& level, std::vector<std::pair<int, int>> pos_) {

        h = level.h;
        w = level.w;
        pos = pos_;

    }

    Tile(const Tile& other) = delete;
    Tile(Tile&& other) = delete;
    Tile& operator=(const Tile& other) = delete;
    Tile& operator=(Tile&& other) = delete;

};

struct Weapon {

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    //unsigned int texture;
    Shader shader;
    int h;
    int w;
    const float duration = 3.f;
    const float blink_freq = 50.0f;
    sf::SoundBuffer soundBuffer;
    sf::Sound sound;

    std::vector<Slot>& grid;
    const Player& nik;
    const Player& ste;


    Weapon(std::vector<Slot>& grid_, const Player& nik_, const Player& ste_) :
        shader("sword"),
        grid(grid_),
        nik(nik_),
        ste(ste_)
    {

        MakeRectWithCoords(30.f / 72.f, 44.f / 72.f, { 279.f / 384.f, 266.f / 369.f }, { 309.f / 384.f, 310.f / 369.f }, VAO, VBO);

        //int width, height;
        //texture = MakeTexture("sword.png", width, height, false, true);

        shader.use();
        glm::mat4 world(1.f);
        world = glm::scale(world, glm::vec3(size, size, 1.f));
        shader.SetMat4("world", world);
        shader.SetMat4("projection", kProjection);

        if (!soundBuffer.loadFromFile(SoundPath("stab.wav"))) {
            std::cerr << "Weapon::Weapon: can't open file \"stab.wav\"\n";
        }
        sound.setBuffer(soundBuffer);
        sound.setVolume(10.f);

    }

    ~Weapon() {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void PlaySound() {
        sound.play();
    }

    void Render() const {

        shader.use();

        glBindTexture(GL_TEXTURE_2D, atlas);
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

        constexpr float smallWeaponScale = 1.f;
        if (nik.armed && (!nik.weaponVanishing || sinf(nik.weapon_t * blink_freq) > -0.2)) {

            glm::mat4 world(1.f);
            world = glm::translate(world, glm::vec3(
                -w / 2.f + size / 2.f + size * nik.precise_x + size / 3.f,
                -h / 2.f + size / 2.f + size * nik.precise_y - size / 8.f,
                0.f
            ));

            world = glm::scale(world, glm::vec3(size * smallWeaponScale, size * smallWeaponScale, 1.f));
            shader.SetMat4("world", world);

            glDrawArrays(GL_TRIANGLES, 0, 6);

        }

        if (ste.armed && (!ste.weaponVanishing || sinf(ste.weapon_t * blink_freq) > -0.2)) {

            glm::mat4 world(1.f);
            world = glm::translate(world, glm::vec3(
                -w / 2.f + size / 2.f + size * ste.precise_x + size / 3.f,
                -h / 2.f + size / 2.f + size * ste.precise_y - size / 8.f,
                0.f
            ));

            world = glm::scale(world, glm::vec3(size * smallWeaponScale, size * smallWeaponScale, 1.f));
            shader.SetMat4("world", world);

            glDrawArrays(GL_TRIANGLES, 0, 6);

        }

    }

    void LoadLevel(const LevelDesc& level) {

        h = level.h;
        w = level.w;

    }

    Weapon(const Weapon& other) = delete;
    Weapon(Weapon&& other) = delete;
    Weapon& operator=(const Weapon& other) = delete;
    Weapon& operator=(Weapon&& other) = delete;

};

struct Ghost {

    enum class Color { Red, Yellow, Blue, Purple, Gray, Brown, Green };
    enum class State { Chase, Scatter, Frightened, Home };

    static const char* const texture_array[5];
    static const float speed_array[5];
    static const float y_array[5];
    static const char* const sound_array[5];
    static const char* const hit_array[5];

    const int size = 1;

    unsigned int VBO;
    unsigned int VAO;
    //unsigned int texture;
    static Shader shader;
    int h;
    int w;

    int x, y;
    int next_x, next_y;
    float precise_x, precise_y;
    int home_x, home_y;
    float t = 0;
    const float baseSpeed;
    float speed;
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

    std::mt19937& mt;

    const Player& nik;
    const Player& ste;

    sf::SoundBuffer soundBuffer;
    sf::Sound sound;

    sf::SoundBuffer hitSoundBuffer;
    sf::Sound hitSound;

    Ghost(Color color_, std::vector<Slot>& grid_, Teleport& teleport_, std::mt19937& mt_, const Player& nik_, const Player& ste_) :
        color(color_),
        state(State::Home),
        baseSpeed(speed_array[static_cast<int>(color_)] * 2.f),
        grid(grid_),
        mt(mt_),
        teleport(teleport_),
        nik(nik_),
        ste(ste_)
    {

        Point a, b;
        a = { 0.f, y_array[static_cast<int>(color)] / 369.f };
        b = { 50.f / 384.f, (y_array[static_cast<int>(color)] + 50.f) / 369.f };

        MakeRectWithCoords(50.f / 72.f, 50.f / 72.f, a, b, VAO, VBO);

        //int width, height;
        //texture = MakeTexture(texture_array[static_cast<int>(color)], width, height, false, true);

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

        if (!soundBuffer.loadFromFile(SoundPath(sound_array[static_cast<int>(color)]))) {
            std::cerr << "Ghost::Ghost: can't open file \"" << sound_array[static_cast<int>(color)] << "\"\n";
        }
        sound.setBuffer(soundBuffer);
        sound.setVolume(25);

        if (!hitSoundBuffer.loadFromFile(SoundPath(hit_array[static_cast<int>(color)]))) {
            std::cerr << "Ghost::Ghost: can't open file \"" << hit_array[static_cast<int>(color)] << "\"\n";
        }
        hitSound.setBuffer(hitSoundBuffer);
        hitSound.setVolume(25);

    }

    Ghost(const Ghost& other) = delete;
    Ghost& operator=(const Ghost& other) = delete;
    Ghost& operator=(Ghost&& other) = delete;

    Ghost(Ghost&& other) :
        size(other.size),
        VBO(other.VBO),
        VAO(other.VAO),
        //texture(other.texture),
        h(other.h),
        w(other.w),
        x(other.x),
        y(other.y),
        next_x(other.next_x),
        next_y(other.next_y),
        precise_x(other.precise_x),
        precise_y(other.precise_y),
        home_x(other.home_x),
        home_y(other.home_y),
        t(other.t),
        speed(other.speed),
        direction(other.direction),
        color(other.color),
        state(other.state),
        target_x(other.target_x),
        target_y(other.target_y),
        scatter_x(other.scatter_x),
        scatter_y(other.scatter_y),
        state_t(other.state_t),
        scatter_duration(other.scatter_duration),
        chase_duration(other.chase_duration),
        frightened_duration(other.frightened_duration),
        home_duration(other.home_duration),
        just_teleported(other.just_teleported),
        grid(other.grid),
        teleport(other.teleport),
        mt(other.mt),
        nik(other.nik),
        ste(other.ste),
        soundBuffer(std::move(other.soundBuffer)),
        sound(std::move(other.sound)),
        hitSoundBuffer(std::move(other.hitSoundBuffer)),
        hitSound(std::move(other.sound)),
        baseSpeed(other.baseSpeed)
    {
        other.VAO = -1;
        other.VBO = -1;
        // TODO: check this trick

        sound.setBuffer(soundBuffer);
        hitSound.setBuffer(hitSoundBuffer);
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

    void ApproachNearest(unsigned char possible_dirs_without_back) {
        float min_dist = std::numeric_limits<float>::max();
        unsigned char min_direction = 16;

        for (unsigned char i = 1; i < 16; i <<= 1) {
            if (possible_dirs_without_back & i) {
                int next_x, next_y;
                DirToNext(i, next_x, next_y);
                const float nik_dist = (next_x - nik.precise_x) * (next_x - nik.precise_x) + (next_y - nik.precise_y) * (next_y - nik.precise_y);
                if (nik_dist < min_dist) {
                    min_dist = nik_dist;
                    min_direction = i;
                }
                if (isSte) {
                    const float ste_dist = (next_x - ste.precise_x) * (next_x - ste.precise_x) + (next_y - ste.precise_y) * (next_y - ste.precise_y);
                    if (ste_dist < min_dist) {
                        min_dist = ste_dist;
                        min_direction = i;
                    }
                }
            }
        }

        direction = min_direction;
    }

    // Update current direction with a random one
    void RandomDirection(unsigned char possible_dirs_without_back, unsigned char n_dirs) {
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

    void SetNewDir(float player_x, float player_y, unsigned char player_dir, float red_x, float red_y) {

        //if (y * w + x < 0 || y * w + x >= grid.size()) {
        //    std::cerr << "Jemel color " << static_cast<int>(color) << " is in cell (" << y << ", " << x << ")\n";
        //    return;
        //}

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

                    if (color == Color::Yellow) {

                        //if (y * w + x < 0 || y * w + x >= grid.size()) {
                        //    std::cerr << "Jemel yellow is in cell (" << y << ", " << x << ")\n";
                        //    return;
                        //}

                        unsigned char player_trace = grid[y * w + x].Direction();
                        if (player_trace & possible_dirs_without_back) {
                            direction = player_trace & possible_dirs_without_back;
                        }
                        else {
                            RandomDirection(possible_dirs_without_back, n_dirs);
                        }
                        return;
                    }

                    else if (color == Color::Blue) {

                        //if (y * w + x < 0 || y * w + x >= grid.size()) {
                        //    std::cerr << "Jemel blue is in cell (" << y << ", " << x << ")\n";
                        //    return;
                        //}

                        unsigned char player_trace = grid[y * w + x].Direction();
                        if (player_trace & possible_dirs_without_back) {
                            direction = player_trace & possible_dirs_without_back;
                        }
                        else {
                            ApproachNearest(possible_dirs_without_back);
                        }

                        return;
                    }

                    else if (color == Color::Purple) {
                        RandomDirection(possible_dirs_without_back, n_dirs);
                        return;
                    }

                    else if (color == Color::Red) {
                        ApproachNearest(possible_dirs_without_back);
                        return;
                    }

                    else if (color == Color::Brown) {
                        // Legacy
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
                    else if (color == Color::Gray) {
                        // Legacy
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
                    else if (color == Color::Green) {
                        // Legacy
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

    void Update(float delta, float red_x, float red_y, bool& hitNik, bool& hitSte) {

        state_t += delta;
        if (state == State::Chase) {
            // Scatter mode does not exist anymore
            //if (state_t >= chase_duration) {
            //    state = State::Scatter;
            //    target_x = scatter_x;
            //    target_y = scatter_y;
            //    state_t = 0;
            //}
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
            //if (y * w + x < 0 || y * w + x >= grid.size()) {
            //    std::cerr << "Jemel color " << static_cast<int>(color) << " is in cell (" << y << ", " << x << ")\n";
            //    return;
            //}
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
            SetNewDir(nik.precise_x, nik.precise_y, nik.direction, red_x, red_y);
            SetNextXY();
        }

        precise_x = x * (1 - t) + next_x * t;
        precise_y = y * (1 - t) + next_y * t;

        // Check collision with player
        constexpr float threshold = 0.1;
        float squaredDistNik = (precise_x - nik.precise_x) * (precise_x - nik.precise_x) + (precise_y - nik.precise_y) * (precise_y - nik.precise_y);
        if (squaredDistNik < threshold) {
            hitNik = true;
            if (!nik.armed && !nik.just_hit) {
                sound.play();
            }
        }

        if (isSte) {
            float squaredDistSte = (precise_x - ste.precise_x) * (precise_x - ste.precise_x) + (precise_y - ste.precise_y) * (precise_y - ste.precise_y);
            if (squaredDistSte < threshold) {
                hitSte = true;
                if (!ste.armed && !ste.just_hit) {
                    sound.play();
                }
            }
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

        float shiftX = ((DirTo2Bit(direction) + 1) * 51.f) / 384.f;
        shader.SetFloat("shiftX", shiftX);

        glBindTexture(GL_TEXTURE_2D, atlas);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    void LoadLevel(const LevelDesc& level, int current_level) {
        h = level.h;
        w = level.w;
        home_x = level.home.front().first;
        home_y = level.home.front().second;
        x = home_x;
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
        else if (color == Color::Purple) {
            scatter_x = w;
            scatter_y = h;
        }
        target_x = scatter_x;
        target_y = scatter_y;
        state_t = 0;
        speed = baseSpeed + baseSpeed * current_level / 40.f;   // magic number

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

//const char* const Ghost::texture_array[5] = { "ghost_red.png", "ghost_yellow.png" , "ghost_blue.png" , "ghost_brown.png", "ghost_purple.png" };
const float Ghost::speed_array[5] = { 1.3f, 1.7f, 1.5f, 1.9f, 1.6f };
const float Ghost::y_array[5] = { 216.f, 267.f, 318.f, 114.f, 165.f };
const char* const Ghost::sound_array[5] = { "numeri.wav", "bam.wav", "buffon.wav", "headshot.wav", "numeri.wav" };
const char* const Ghost::hit_array[5] = { "barbani.wav", "berta.wav", "onesto.wav", "berta.wav", "barbani.wav" };
Shader Ghost::shader;

//const char* const Player::texture_array[2] = { "nik.png", "ste.png" };

int Player::lives;

#endif // NIKMAN_ENTITY_H