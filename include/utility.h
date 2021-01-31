#if !defined NIKMAN_UTILITY_H
#define NIKMAN_UTILITY_H

#include <vector>
#include <fstream>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr char* const kShaderRoot = "../shaders";
static constexpr char* const kTextureRoot = "../resources";
static constexpr char* const kLevelRoot = "../resources/levels";

static constexpr float kRatio = 16.f / 9.f;
static constexpr float kWorldHeight = 11.f;  // It shall be higher in production
static constexpr float kWorldWidth = kWorldHeight * kRatio;
static constexpr int kWindowWidth = 1600;
static constexpr int kWindowHeight = kWindowWidth / kRatio;

static const glm::mat4 kProjection = glm::ortho(-kWorldWidth / 2, kWorldWidth / 2, -kWorldHeight / 2, kWorldHeight / 2, 0.1f, 10.f);

unsigned int MakeTexture(const char* filename, int& width, int& height, bool nearest = false, bool alpha = false) {
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
    int nrChannels;

    std::filesystem::path texture_path = std::filesystem::path(kTextureRoot) / std::filesystem::path(filename);
    std::string texture_path_string = texture_path.string();
    const char* texture_path_cstring = texture_path_string.c_str();

    unsigned char* data = stbi_load(texture_path_cstring, &width, &height, &nrChannels, 0);
    if (data)
    {
        //GLenum format = alpha ? GL_RGBA : GL_RGB;
        GLint internal_format = alpha ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}


#endif // NIKMAN_UTILITY_H