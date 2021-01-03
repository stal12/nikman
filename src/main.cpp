#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "utility.h"
#include "shader.h"
#include "entity.h"

// WARNING: glfw and glad are currently in Debug mode



// This callback should be called each time the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// Close the window with ESC
void processInput(GLFWwindow* window, unsigned int& wasd)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    wasd = 0;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        wasd |= 1;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        wasd |= 2;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        wasd |= 4;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        wasd |= 8;

}



int main()
{
    constexpr int window_width = 1600;
    constexpr int window_height = window_width / kRatio;

    // Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Nikman", NULL, NULL);
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
    glViewport(0, 0, window_width, window_height);
    glfwSetFramebufferSizeCallback(window,
        [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        }
    );
    
    stbi_set_flip_vertically_on_load(true);
    
    {

        Map map;
        Wall wall(map.size);
        Crust crust(map.size, map.grid);
        Player player(map.size, map.grid);
        map.FillWalls(wall.ver_positions, wall.hor_positions);

        // Very simple render loop
        float formerFrame = glfwGetTime();
        while (!glfwWindowShouldClose(window))
        {
            float currentFrame = glfwGetTime();
            float delta = currentFrame - formerFrame;
            formerFrame = currentFrame;

            // Input
            unsigned int wasd;
            processInput(window, wasd);

            // Update
            player.Update(delta, wasd);

            // Render
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            map.Render();
            wall.Render();
            crust.Render();
            player.Render();

            // check and call events and swap the buffers
            glfwPollEvents();
            glfwSwapBuffers(window);
        }

        // Clean/Delete all of GLFW's resources that were allocated
    }
    glfwTerminate();
    return 0;
}