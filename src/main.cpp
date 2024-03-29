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


int main()
{

    // Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window object
    glfwWindowHint(GLFW_SAMPLES, 4);    // MSAA
    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Nikman", glfwGetPrimaryMonitor(), NULL);
    //GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Nikman", NULL, NULL);
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
        Game game;
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        //glEnable(GL_FRAMEBUFFER_SRGB);

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
            game.Update(delta, wasd, stop_game);

            // Render
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            game.Render();

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