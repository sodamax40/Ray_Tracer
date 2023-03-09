#pragma once

#include <GL/glew.h>
#include <lodepng.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace cg {

std::string readShaderSource(const std::string &filename)
{
    std::ifstream file(filename);
    std::stringstream stream;
    stream << file.rdbuf();

    return stream.str();
}

void showShaderInfoLog(GLuint shader)
{
    GLint infoLogLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> infoLog(infoLogLength);
    glGetShaderInfoLog(shader, infoLogLength, &infoLogLength, &infoLog[0]);
    std::string infoLogStr(infoLog.begin(), infoLog.end());
    std::cerr << infoLogStr << std::endl;
}

void showProgramInfoLog(GLuint program)
{
    GLint infoLogLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> infoLog(infoLogLength);
    glGetProgramInfoLog(program, infoLogLength, &infoLogLength, &infoLog[0]);
    std::string infoLogStr(infoLog.begin(), infoLog.end());
    std::cerr << infoLogStr << std::endl;
}

GLuint loadShaderProgram(const std::string &vertexShaderFilename,
                         const std::string &fragmentShaderFilename)
{
    // Load and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderSource = readShaderSource(vertexShaderFilename);
    const char *vertexShaderSourcePtr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourcePtr, nullptr);

    glCompileShader(vertexShader);
    GLint compiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cerr << "Vertex shader compilation failed:" << std::endl;
        showShaderInfoLog(vertexShader);
        glDeleteShader(vertexShader);
        return 0;
    }

    // Load and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderSource = readShaderSource(fragmentShaderFilename);
    const char *fragmentShaderSourcePtr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourcePtr, nullptr);

    glCompileShader(fragmentShader);
    compiled = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cerr << "Fragment shader compilation failed:" << std::endl;
        showShaderInfoLog(fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Create program object
    GLuint program = glCreateProgram();

    // Attach shaders to the program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // Link program
    glLinkProgram(program);

    // Check linking status
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        std::cerr << "Linking failed:" << std::endl;
        showProgramInfoLog(program);
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Clean up
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    return program;
}

GLuint load2DTexture(const std::string &filename)
{
    std::vector<unsigned char> data;
    unsigned width, height;
    unsigned error = lodepng::decode(data, width, height, filename);
    if (error != 0) {
        std::cout << "Error: " << lodepng_error_text(error) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, &(data[0]));
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

// Load cubemap texture and let OpenGL generate a mipmap chain
GLuint loadCubemap(const std::string &dirname)
{
    const char *filenames[] = { "posx.png", "negx.png", "posy.png", "negy.png", "posz.png", "negz.png" };
    const GLenum targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    const unsigned num_sides = 6; 

    std::vector<unsigned char> data[num_sides];
    unsigned width;
    unsigned height;
    for (unsigned i = 0; i < num_sides; ++i) {
        std::string filename = dirname + "/" + filenames[i];
        unsigned error = lodepng::decode(data[i], width, height, filename);
        if (error != 0) {
            std::cout << "Error: " << lodepng_error_text(error) << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    for (unsigned i = 0; i < num_sides; ++i) {
        glTexImage2D(targets[i], 0, GL_SRGB8_ALPHA8, width, height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, &(data[i][0]));
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return texture;
}

// Load cubemap with pre-computed mipmap chain
GLuint loadCubemapMipmap(const std::string &dirname)
{
    const char *levels[] = { "2048", "512", "128", "32", "8", "2", "0.5", "0.125" };
    const char *filenames[] = { "posx.png", "negx.png", "posy.png", "negy.png", "posz.png", "negz.png" };
    const GLenum targets[] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    const unsigned num_levels = sizeof(levels) / sizeof(levels[0]);
    const unsigned num_sides = 6; 

    std::vector<unsigned char> data[num_levels][num_sides];
    unsigned width[num_levels];
    unsigned height[num_levels];
    for (unsigned i = 0; i < num_levels; ++i) {
        for (unsigned j = 0; j < num_sides; ++j) {
            std::string filename = dirname + "/" + levels[i] + "/" + filenames[j];
            unsigned error = lodepng::decode(data[i][j], width[i], height[i], filename);
            if (error != 0) {
                std::cout << "Error: " << lodepng_error_text(error) << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    for (unsigned i = 0; i < num_levels; ++i) {
        for (unsigned j = 0; j < num_sides; ++j) {
            glTexImage2D(targets[j], i, GL_SRGB8_ALPHA8, width[i], height[i],
                         0, GL_RGBA, GL_UNSIGNED_BYTE, &(data[i][j][0]));
        }
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return texture;
}

}  // namespace cg
