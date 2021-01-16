#if !defined NIKMAN_SHADER_H
#define NIKMAN_SHADER_H

#include <filesystem>
#include <string>
#include <iostream>
#include <fstream>

#include <glad/glad.h>

#include "utility.h"

enum class ShaderType { Vertex, Fragment, Geometry, None };

//! Creates a shader from a file
/*!
  \param filename file containing the shader source code
  \param type Shader type, or ShaderType::None, in which case the type is deduced from file extension
  \return the created shader id, or -1 in case of failure
*/
unsigned int CreateShader(const char* filename, ShaderType type = ShaderType::None) {

    if (type == ShaderType::None) {
        int len = strlen(filename);
        if (strcmp(filename + len - 5, ".vert") == 0)
            type = ShaderType::Vertex;
        else if (strcmp(filename + len - 5, ".frag") == 0)
            type = ShaderType::Fragment;
        else if (strcmp(filename + len - 5, ".geom") == 0)
            type = ShaderType::Geometry;
        else {
            std::cerr << "Error! Unknown shader type.\n";
            return -1;
        }
    }

    unsigned int shader;
    GLenum shader_type;
    std::string name;
    switch (type) {
    case ShaderType::Vertex: 
        shader_type = GL_VERTEX_SHADER; 
        name = "Vertex";
        break;
    case ShaderType::Fragment: 
        shader_type = GL_FRAGMENT_SHADER; 
        name = "Fragment";
        break;
    case ShaderType::Geometry: 
        shader_type = GL_GEOMETRY_SHADER; 
        name = "Geometry";
        break;
    default:    
        return -1;
    }

    shader = glCreateShader(shader_type);

    {
        std::ifstream is(std::filesystem::path(kShaderRoot) / std::filesystem::path(filename), std::ios::binary);
        if (!is.is_open()) {
            glDeleteShader(shader);
            std::cerr << "Error! Can't open shader source code.\n";
            return -1;
        }

        char* shader_source;
        is.seekg(0, std::ios_base::end);
        int file_size = is.tellg();
        is.seekg(0, std::ios_base::beg);
        shader_source = new char[file_size + 1];
        is.read(shader_source, file_size);
        shader_source[file_size] = 0;
        glShaderSource(shader, 1, &shader_source, NULL);
        delete[] shader_source;
    }

    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Error! " << name << " shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return -1;
    }

    return shader;
}


struct Shader {

    unsigned int program;

    Shader(const char* vertex_file, const char* fragment_file, const char* geometry_file = nullptr) {

        const unsigned int vertex = CreateShader(vertex_file);
        const unsigned int fragment = CreateShader(fragment_file);
        const unsigned int geometry = geometry_file != nullptr ? CreateShader(geometry_file) : -1;

        program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        if (geometry_file != nullptr) {
            glAttachShader(program, geometry);
        }
        glLinkProgram(program);

        int success;
        char infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Error! Shader program linking failed: " << infoLog << std::endl;
            glDeleteProgram(program);
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometry_file != nullptr) {
            glDeleteShader(geometry);
        }

    }

    Shader(const char* prefix, bool geometry = false) : Shader(
        (prefix + std::string(".vert")).c_str(), 
        (prefix + std::string(".frag")).c_str(),
        geometry ? (prefix + std::string(".geom")).c_str() : nullptr) {}

    void use() const {
        glUseProgram(program);
    }

    ~Shader() {
        glDeleteProgram(program);
    }

    void SetMat4(const char* key, const glm::mat4& value) const {
        glUniformMatrix4fv(glGetUniformLocation(program, key), 1, GL_FALSE, glm::value_ptr(value));
    }

    void SetVec3(const char* key, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(program, key), 1, glm::value_ptr(value));
    }

    void SetFloat(const char* key, float value) const {
        glUniform1f(glGetUniformLocation(program, key), value);
    }

};

#endif SHADER_H