#if !defined NIKMAN_UTILITY_H
#define NIKMAN_UTILITY_H

#include <vector>
#include <fstream>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static bool isSte = false;
static unsigned int atlas;

static constexpr char* const kShaderRoot = "../shaders";
static constexpr char* const kTextureRoot = "../resources/textures";
static constexpr char* const kLevelRoot = "../resources/levels";
static constexpr char* const kSoundsRoot = "../resources/sounds";
static constexpr char* const kFontRoot = "../resources/fonts";
static constexpr char* const kScoresPath = "highscores.txt";

static constexpr float kRatio = 16.f / 9.f;
static constexpr float kWorldHeight = 15.f;  // It shall be higher in production
static constexpr float kWorldWidth = kWorldHeight * kRatio;
static constexpr int kWindowWidth = 1920;
static constexpr int kWindowHeight = kWindowWidth / kRatio;
static constexpr float kVerticalShift = 0.3f;  // kWorldHeight / 20.f;

static const glm::mat4 kProjection = glm::ortho(
    -kWorldWidth / 2.f,
    kWorldWidth / 2.f, 
    (-kWorldHeight / 2.f) + kVerticalShift, 
    (kWorldHeight / 2.f) + kVerticalShift, 
    0.1f, 
    10.f);

unsigned int MakeTextureGeneral(const char* filename, int& width, int& height, bool nearest = false, bool alpha = false) {
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

    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
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

unsigned int MakeTexture(const char* filename, int& width, int& height, bool nearest = false, bool alpha = false) {

    std::filesystem::path texture_path = std::filesystem::path(kTextureRoot) / std::filesystem::path(filename);
    std::string texture_path_string = texture_path.string();
    const char* texture_path_cstring = texture_path_string.c_str();

    return MakeTextureGeneral(texture_path_cstring, width, height, nearest, alpha);
}

unsigned int MakeFontTexture(const char* filename, int& width, int& height, bool nearest = false, bool alpha = false) {

    std::filesystem::path texture_path = std::filesystem::path(kFontRoot) / std::filesystem::path(filename);
    std::string texture_path_string = texture_path.string();
    const char* texture_path_cstring = texture_path_string.c_str();

    return MakeTextureGeneral(texture_path_cstring, width, height, nearest, alpha);
}

std::string SoundPath(const char* name) {
    return (std::filesystem::path(kSoundsRoot) / std::filesystem::path(name)).string();
}

std::string TexturePath(const char* name) {
    return (std::filesystem::path(kTextureRoot) / std::filesystem::path(name)).string();
}

std::string FontPath(const char* name) {
    return (std::filesystem::path(kFontRoot) / std::filesystem::path(name)).string();
}

unsigned char DirTo2Bit(unsigned char dir) {
    if (dir == 1) return 0;
    else if (dir == 2) return 1;
    else if (dir == 4) return 2;
    else return 3;
}

#endif // NIKMAN_UTILITY_H