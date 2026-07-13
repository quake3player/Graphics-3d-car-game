#include "Engine.h"
#include <iostream>

void Engine::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Engine& e = Engine::get();
    e.m_width = width;
    e.m_height = height;
    glViewport(0, 0, width, height);
}

bool Engine::init(int width, int height, const char* title) {
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    m_width = width;
    m_height = height;

    // Set the framebuffer size callback so the viewport resizes with the window
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

    glEnable(GL_DEPTH_TEST);

    return true;
}

void Engine::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool Engine::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Engine::beginFrame() {
    glClearColor(0.53f, 0.74f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::endFrame() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool Engine::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}
