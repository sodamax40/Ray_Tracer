// Project 1: Ray tracing (1TD388 and 1MD150)
//
// Modify this file and other files according to the instructions.
//

#include "rt_raytracing.h"
#include "cg_utils.h"
#include "cg_utils2.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>
#include <algorithm>

// Struct for resources and state
struct Context
{
    int width;
    int height;
    float aspect;
    GLFWwindow *window;
    GLuint program;
    cg::Trackball trackball;
    GLuint emptyVAO;
    rt::RTContext rtx;
    GLuint texture = 0;
    float elapsed_time;
};

// Returns the value of an environment variable
std::string getEnvVar(const std::string &name)
{
    char const *value = std::getenv(name.c_str());
    if (value == nullptr)
    {
        return std::string();
    }
    else
    {
        return std::string(value);
    }
}

// Returns the absolute path to the shader directory
std::string shaderDir(void)
{
    std::string rootDir = getEnvVar("RT_VIEWER_ROOT");
    if (rootDir.empty())
    {
        std::cout << "Error: RT_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/src/shaders/";
}

// Returns the absolute path to the 3D model directory
std::string modelDir(void)
{
    std::string rootDir = getEnvVar("RT_VIEWER_ROOT");
    if (rootDir.empty())
    {
        std::cout << "Error: RT_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/3d_models/";
}

void createImageTexture(GLuint *texture, int width, int height)
{
    glDeleteTextures(1, texture);
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initializeTrackball(Context &ctx)
{
    double radius = double(std::min(ctx.width, ctx.height)) / 2.0;
    ctx.trackball.radius = radius;
    glm::vec2 center = glm::vec2(ctx.width, ctx.height) / 2.0f;
    ctx.trackball.center = center;
}

void init(Context &ctx)
{
    ctx.program =
        cg::loadShaderProgram(shaderDir() + "draw_image.vert", shaderDir() + "draw_image.frag");
    createImageTexture(&ctx.texture, ctx.rtx.width, ctx.rtx.height);

    // Set up ray tracing scene
    rt::setupScene(ctx.rtx, (modelDir() + "bunny_lowpoly.obj").c_str());

    initializeTrackball(ctx);
}

void updateRayTracing(Context &ctx)
{
    // Check for convergence
    if (ctx.rtx.current_frame >= ctx.rtx.max_frames)
    {
        return;
    }

    // Render as much as we can within the current frame
    float tic = glfwGetTime();
    while (true)
    {
        rt::updateImage(ctx.rtx);
        if (glfwGetTime() - tic > (1.0f / 60.0f))
        {
            break;
        }
    }
}

void drawImage(Context &ctx)
{
    // Bind texture and upload new image from the ray tracing
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ctx.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ctx.rtx.width, ctx.rtx.height, 0, GL_RGBA, GL_FLOAT,
                 &ctx.rtx.image[0]);

    // Activate program and pass uniform for texture unit
    glUseProgram(ctx.program);
    glUniform1i(glGetUniformLocation(ctx.program, "u_texture"), 0);

    // Draw fullscreen quad (without any vertex buffers)
    glBindVertexArray(ctx.emptyVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// MODIFY THIS FUNCTION
void showGui(Context &ctx)
{
    if (ImGui::SliderInt("Max bounces", &ctx.rtx.max_bounces, 0, 10))
    {
        rt::resetAccumulation(ctx.rtx);
    }
    if (ImGui::ColorEdit3("Sky color", &ctx.rtx.sky_color[0]))
    {
        rt::resetAccumulation(ctx.rtx);
    }
    if (ImGui::ColorEdit3("Ground color", &ctx.rtx.ground_color[0]))
    {
        rt::resetAccumulation(ctx.rtx);
    }
    if (ImGui::Checkbox("Show normals", &ctx.rtx.show_normals))
    {
        rt::resetAccumulation(ctx.rtx);
    }
    // Add more settings and parameters here
    // ...
    if (ImGui::Checkbox("Show Gamma correction", &ctx.rtx.gamma_correction))
    {
        rt::resetAccumulation(ctx.rtx);
        glUniform1i(glGetUniformLocation(ctx.program, "u_gamma_correction"), ctx.rtx.gamma_correction);
    }

    if (ImGui::Checkbox("Show Anti-aliasing", &ctx.rtx.anti_aliasing))
    {
        rt::resetAccumulation(ctx.rtx);
    }

    if (ImGui::SliderFloat("Metal Fuzz", &ctx.rtx.fuzz, 0.0, 1.0))
    {
        rt::resetAccumulation(ctx.rtx);
    }

    ImGui::Text("Progress");
    ImGui::ProgressBar(float(ctx.rtx.current_frame) / ctx.rtx.max_frames);
    if (ImGui::Button("Freeze/Resume"))
    {
        ctx.rtx.freeze = !ctx.rtx.freeze;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        rt::resetImage(ctx.rtx);
    }
}

void display(Context &ctx)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Update view matrix
    glm::mat4 trackball = cg::trackballGetRotationMatrix(ctx.trackball);
    glm::vec3 eye = glm::mat3(trackball) * glm::vec3(0.0f, 0.0f, 2.0f);
    ctx.rtx.view = glm::lookAt(eye, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    if (ctx.trackball.tracking)
    {
        rt::resetAccumulation(ctx.rtx);
    }

    // Update and draw ray tracing image
    updateRayTracing(ctx);
    drawImage(ctx);

    showGui(ctx);
}

void reloadShaders(Context *ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program =
        cg::loadShaderProgram(shaderDir() + "draw_image.vert", shaderDir() + "draw_image.frag");
}

void mouseButtonPressed(Context *ctx, int button, int x, int y)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        ctx->trackball.center = glm::vec2(x, y);
        cg::trackballStartTracking(ctx->trackball, glm::vec2(x, y));
    }
}

void mouseButtonReleased(Context *ctx, int button, int x, int y)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        cg::trackballStopTracking(ctx->trackball);
    }
}

void moveTrackball(Context *ctx, int x, int y)
{
    if (ctx->trackball.tracking)
    {
        cg::trackballMove(ctx->trackball, glm::vec2(x, y));
    }
}

void errorCallback(int /*error*/, const char *description)
{
    std::cerr << description << std::endl;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Forward event to GUI
    ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard)
    {
        return;
    } // Skip other handling

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        reloadShaders(ctx);
    }
}

void charCallback(GLFWwindow *window, unsigned int codepoint)
{
    // Forward event to GUI
    ImGui_ImplGlfwGL3_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantTextInput)
    {
        return;
    } // Skip other handling
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    // Forward event to GUI
    ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    } // Skip other handling

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS)
    {
        mouseButtonPressed(ctx, button, x, y);
    }
    else
    {
        mouseButtonReleased(ctx, button, x, y);
    }
}

void cursorPosCallback(GLFWwindow *window, double x, double y)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    } // Skip other handling

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    moveTrackball(ctx, x, y);
}

void resizeCallback(GLFWwindow *window, int width, int height)
{
    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->width = width;
    ctx->height = height;
    ctx->aspect = float(width) / float(height);
    ctx->trackball.radius = double(std::min(width, height)) / 2.0;
    ctx->trackball.center = glm::vec2(width, height) / 2.0f;
    glViewport(0, 0, width, height);

    ctx->rtx.width = width;
    ctx->rtx.height = height;
    rt::resetImage(ctx->rtx);
}

int main(void)
{
    Context ctx;

    // Create a GLFW window
    glfwSetErrorCallback(errorCallback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.width = 500;
    ctx.height = 500;
    ctx.aspect = float(ctx.width) / float(ctx.height);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Ray tracer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, keyCallback);
    glfwSetCharCallback(ctx.window, charCallback);
    glfwSetMouseButtonCallback(ctx.window, mouseButtonCallback);
    glfwSetCursorPosCallback(ctx.window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(ctx.window, resizeCallback);

    // Load OpenGL functions
    glewExperimental = true;
    GLenum status = glewInit();
    if (status != GLEW_OK)
    {
        std::cerr << "Error: " << glewGetErrorString(status) << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize GUI
    ImGui_ImplGlfwGL3_Init(ctx.window, false /*do not install callbacks*/);

    // Initialize rendering
    glGenVertexArrays(1, &ctx.emptyVAO);
    glBindVertexArray(ctx.emptyVAO);
    init(ctx);

    // Start rendering loop
    while (!glfwWindowShouldClose(ctx.window))
    {
        glfwPollEvents();
        ctx.elapsed_time = glfwGetTime();
        ImGui_ImplGlfwGL3_NewFrame();
        display(ctx);
        ImGui::Render();
        glfwSwapBuffers(ctx.window);
    }

    // Shutdown
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
