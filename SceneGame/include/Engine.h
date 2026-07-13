#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Engine {
public:
    static Engine& get() {
        static Engine instance;
        return instance;
    }

    bool init(int width, int height, const char* title);
    void shutdown();
    
    bool shouldClose() const;
    void beginFrame();
    void endFrame();
    
    GLFWwindow* getWindow() const { return m_window; }
    
    bool isKeyPressed(int key) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    Engine() = default;
    ~Engine() = default;
    
    GLFWwindow* m_window = nullptr;
    int m_width = 0;
    int m_height = 0;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
