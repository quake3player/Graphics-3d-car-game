#pragma once
#include "game_runtime.h"

bool checkCollision(const GameRuntime& rt, float nextX, float nextZ);
void updatePhysics(GameRuntime& rt, float deltaTime);
