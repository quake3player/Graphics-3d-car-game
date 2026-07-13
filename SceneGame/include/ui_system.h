#pragma once
#include "game_runtime.h"
#include <GLFW/glfw3.h>

void initImGui(GLFWwindow* window);
void renderUI(GameRuntime& rt, float fps, bool& bulletTime);
void cleanupImGui();
