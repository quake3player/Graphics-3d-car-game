#pragma once
#include "game_runtime.h"
#include <GLFW/glfw3.h>

void handleInput(GLFWwindow* window, GameRuntime& rt, float deltaTime, bool& bulletTimeActive);
