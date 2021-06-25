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

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include <SFML/Audio.hpp>

#include "utility.h"
#include "shader.h"
#include "entity.h"
#include "level.h"
#include "game.h"
#include "ui.h"

// TODO this worked once, and then no more
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup") 

using namespace std::filesystem;


// WARNING: glfw and glad are currently in Debug mode


// This callback should be called each time the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// Close the window with ESC
void processInput(GLFWwindow* window, unsigned int& wasd)
{
    //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    //    glfwSetWindowShouldClose(window, true);

    wasd = 0;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        wasd |= 1;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        wasd |= 2;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        wasd |= 4;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        wasd |= 8;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        wasd |= 16;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        wasd |= 32;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        wasd |= 64;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        wasd |= 128;
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        wasd |= 256;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        wasd |= 512;

}


LevelDesc GenerateLevel(int h, int w, std::mt19937& mt) {

    LevelDesc level;

    level.h = h;
    level.w = w;

    std::vector<bool> hor_walls((h + 1) * w, true);
    std::vector<bool> ver_walls(h * (w + 1), true);

#define DOWN(x, y) (hor_walls[y*w+x])
#define UP(x, y) (hor_walls[(y+1)*w+x])
#define LEFT(x, y) (ver_walls[y*(w+1)+x])
#define RIGHT(x, y) (ver_walls[y*(w+1)+x+1])

    // Set borders
    //for (int y = 0; y < h; ++y) {
    //    LEFT(0, y) = true;
    //    RIGHT(w - 1, y) = true;
    //}

    //for (int x = 0; x < w; ++x) {
    //    DOWN(x, 0) = true;
    //    UP(x, h - 1) = true;
    //}

    std::vector<int> visited(h * w, false);

    std::uniform_int_distribution x_dis(0, w - 1);
    std::uniform_int_distribution y_dis(0, h - 1);
    std::uniform_int_distribution dir_dis(0, 3);

    // Start from random cell
    int x = x_dis(mt);
    int y = y_dis(mt);

    int prev_dir = -1;
    int dir;

    while (!visited[y * w + x]) {
        visited[y * w + x] = true;

        do {
            dir = dir_dis(mt);
        } while ((x == 0 && dir == 1) || (x == w - 1 && dir == 3) || (y == 0 && dir == 2) || (y == h - 1 && dir == 0) || (dir == prev_dir));

        if (dir == 0) {
            UP(x, y) = false;
            ++y;
            prev_dir = 2;
        }
        else if (dir == 2) {
            DOWN(x, y) = false;
            --y;
            prev_dir = 0;
        }
        else if (dir == 1) {
            LEFT(x, y) = false;
            --x;
            prev_dir = 3;
        }
        else {
            RIGHT(x, y) = false;
            ++x;
            prev_dir = 1;
        }
    }

    int r = 0;
    while (r < 2) {

        do {
            x = x_dis(mt);
            y = y_dis(mt);
        } while (!visited[y * w + x]);

        do {
            visited[y * w + x] = true;

            do {
                dir = dir_dis(mt);
            } while ((x == 0 && dir == 1) || (x == w - 1 && dir == 3) || (y == 0 && dir == 2) || (y == h - 1 && dir == 0) || (dir == prev_dir));

            if (dir == 0) {
                UP(x, y) = false;
                ++y;
                prev_dir = 2;
            }
            else if (dir == 2) {
                DOWN(x, y) = false;
                --y;
                prev_dir = 0;
            }
            else if (dir == 1) {
                LEFT(x, y) = false;
                --x;
                prev_dir = 3;
            }
            else {
                RIGHT(x, y) = false;
                ++x;
                prev_dir = 1;
            }
        } while (!visited[y * w + x]);

        ++r;
    }


    // Make level desc 
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w + 1; ++x) {
            if (ver_walls[y * (w + 1) + x]) {
                level.ver_walls.emplace_back(x, y);
            }
        }
    }
    for (int y = 0; y < h + 1; ++y) {
        for (int x = 0; x < w; ++x) {
            if (hor_walls[y * w + x]) {
                level.hor_walls.emplace_back(x, y);
            }
        }
    }

    return level;

}


int main()
{

    //LevelDesc level = ReadLevelDesc((std::filesystem::path(kLevelRoot) / std::filesystem::path("level.txt")).string().c_str()); // TODO remove this

    // Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window object
    glfwWindowHint(GLFW_SAMPLES, 4);    // MSAA
    //GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Nikman", glfwGetPrimaryMonitor(), NULL);
    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Nikman", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set the size of the rendering window, and set a callback for the window resize event
    glViewport(0, 0, kWindowWidth, kWindowHeight);
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        }
    );
    
    stbi_set_flip_vertically_on_load(true);
    
    int height, width;
    atlas = MakeTexture("atlas.png", width, height, true, true);   // it would be better to use RAII

    {
        std::random_device rd;
        std::mt19937 mt(rd());

        Map map;
        Wall wall;

        LevelDesc level = GenerateLevel(14, 26, mt);

        map.LoadLevel(level, mt);
        wall.LoadLevel(level);

        Game game;
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glEnable(GL_FRAMEBUFFER_SRGB);

        // Very simple render loop
        float formerFrame = glfwGetTime();
        bool stop_game = false;
        while (!glfwWindowShouldClose(window) && !stop_game)
        {
            float currentFrame = glfwGetTime();
            float delta = currentFrame - formerFrame;
            formerFrame = currentFrame;

            // Input
            unsigned int wasd;
            processInput(window, wasd);

            // Update
            //game.Update(delta, wasd, stop_game);

            // Render
            glClearColor(0.01f, 0.01f, 0.01f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            map.Render();
            wall.Render();
            //game.Render();

            // check and call events and swap the buffers
            glfwPollEvents();
            glfwSwapBuffers(window);
        }

        // Clean/Delete all of GLFW's resources that were allocated

        glDeleteTextures(1, &atlas);
    }
    glfwTerminate();
    return 0;
}