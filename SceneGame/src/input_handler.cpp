#include "input_handler.h"
#include "game_logic.h"
#include <iostream>
#include <cmath>

void handleInput(GLFWwindow* window, GameRuntime& rt, float deltaTime, bool& bulletTimeActive) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Reset — 'x' key
    static bool xWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (!xWasPressed) { resetWorld(rt); xWasPressed = true; }
    } else { xWasPressed = false; }

    // Camera cycle: 'c'
    static bool cWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cWasPressed) { rt.cameraIndex = (rt.cameraIndex + 1) % 5; cWasPressed = true; }
    } else { cWasPressed = false; }

    // Reverse toggle: 'r' (one-shot)
    static bool rWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!rWasPressed) { rt.reverseMode = !rt.reverseMode; rWasPressed = true; }
    } else { rWasPressed = false; }

    // Bullet time toggle: 'b'
    static bool bWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (!bWasPressed) { bulletTimeActive = !bulletTimeActive; bWasPressed = true; }
    } else { bWasPressed = false; }

    // Gear up: 'g' (one-shot)
    static bool gWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (!gWasPressed && rt.currentGear < 4) { rt.currentGear++; gWasPressed = true; }
    } else { gWasPressed = false; }

    // Gear down: 'h' (one-shot)
    static bool hWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        if (!hWasPressed && rt.currentGear > 1) { rt.currentGear--; hWasPressed = true; }
    } else { hWasPressed = false; }

    // Light source selection: 1/2/3/4 keys (for Light View)
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) rt.selectedLight = 0;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) rt.selectedLight = 1;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) rt.selectedLight = 2;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) rt.selectedLight = 3;

    // Zoom: '+' / '=' to zoom in, '-' to zoom out
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        rt.zoomFov -= konst::kZoomStep * deltaTime * 30.0f;
        if (rt.zoomFov < konst::kMinZoom) rt.zoomFov = konst::kMinZoom;
    }
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
        rt.zoomFov += konst::kZoomStep * deltaTime * 30.0f;
        if (rt.zoomFov > konst::kMaxZoom) rt.zoomFov = konst::kMaxZoom;
    }

    // Daylight: '[' to decrease, ']' to increase
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
        rt.daylightLevel -= konst::kDaylightStep * deltaTime;
        if (rt.daylightLevel < 0.0f) rt.daylightLevel = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
        rt.daylightLevel += konst::kDaylightStep * deltaTime;
        if (rt.daylightLevel > 1.0f) rt.daylightLevel = 1.0f;
    }

    // Windmill speed: P = speed up, O = speed down
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        for (auto& b : rt.buildings) {
            b.windmillSpeedMultiplier += konst::kWindmillSpeedStep * deltaTime;
            if (b.windmillSpeedMultiplier > konst::kWindmillMaxMult) b.windmillSpeedMultiplier = konst::kWindmillMaxMult;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        for (auto& b : rt.buildings) {
            b.windmillSpeedMultiplier -= konst::kWindmillSpeedStep * deltaTime;
            if (b.windmillSpeedMultiplier < 0.0f) b.windmillSpeedMultiplier = 0.0f;
        }
    }

    // ── Camera Controls: Arrow keys ──
    bool camL = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
    bool camR = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
    bool camU = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    bool camD = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

    switch (rt.cameraIndex) {
        case 0: // Sky view — pan XZ
            if (camL) rt.skyOffsetX -= konst::kCamMoveSpeed * deltaTime;
            if (camR) rt.skyOffsetX += konst::kCamMoveSpeed * deltaTime;
            if (camU) rt.skyOffsetZ -= konst::kCamMoveSpeed * deltaTime;
            if (camD) rt.skyOffsetZ += konst::kCamMoveSpeed * deltaTime;
            break;
        case 1: // Car view — yaw/pitch
            if (camL) rt.carCamYaw += konst::kCamRotSpeed * deltaTime;
            if (camR) rt.carCamYaw -= konst::kCamRotSpeed * deltaTime;
            if (camU) rt.carCamPitch += konst::kCamRotSpeed * deltaTime;
            if (camD) rt.carCamPitch -= konst::kCamRotSpeed * deltaTime;
            if (rt.carCamPitch > konst::kCamPitchLimit) rt.carCamPitch = konst::kCamPitchLimit;
            if (rt.carCamPitch < -konst::kCamPitchLimit) rt.carCamPitch = -konst::kCamPitchLimit;
            break;
        case 2: // Ground view — yaw/pitch
            if (camL) rt.groundCamYaw -= konst::kCamRotSpeed * deltaTime;
            if (camR) rt.groundCamYaw += konst::kCamRotSpeed * deltaTime;
            
            // Clamp yaw to +/- 30 degrees (pi/6) from base yaw (pi)
            if (rt.groundCamYaw > konst::kPi + konst::kPi / 6.0f) rt.groundCamYaw = konst::kPi + konst::kPi / 6.0f;
            if (rt.groundCamYaw < konst::kPi - konst::kPi / 6.0f) rt.groundCamYaw = konst::kPi - konst::kPi / 6.0f;

            if (camU) rt.groundCamPitch += konst::kCamRotSpeed * deltaTime;
            if (camD) rt.groundCamPitch -= konst::kCamRotSpeed * deltaTime;
            if (rt.groundCamPitch > konst::kCamPitchLimit) rt.groundCamPitch = konst::kCamPitchLimit;
            if (rt.groundCamPitch < -konst::kCamPitchLimit) rt.groundCamPitch = -konst::kCamPitchLimit;
            break;
        case 3: // Light view — no manual rotation (follows light)
            break;
        case 4: // Helicopter — Up/Down = height, Left/Right = distance
            if (camU) rt.heliHeight += konst::kCamMoveSpeed * 0.5f * deltaTime;
            if (camD) rt.heliHeight -= konst::kCamMoveSpeed * 0.5f * deltaTime;
            if (camL) rt.heliDist += konst::kCamMoveSpeed * 0.5f * deltaTime;
            if (camR) rt.heliDist -= konst::kCamMoveSpeed * 0.5f * deltaTime;
            if (rt.heliHeight < konst::kHeliMinHeight) rt.heliHeight = konst::kHeliMinHeight;
            if (rt.heliHeight > konst::kHeliMaxHeight) rt.heliHeight = konst::kHeliMaxHeight;
            if (rt.heliDist < konst::kHeliMinDist) rt.heliDist = konst::kHeliMinDist;
            if (rt.heliDist > konst::kHeliMaxDist) rt.heliDist = konst::kHeliMaxDist;
            break;
    }

    // If car is stopped (collision or breakdown), block all movement inputs
    if (rt.carStopped || rt.carBrokenDown) return;

    // ── Car Controls: WASD only ──
    // W = Throttle (increase speed magnitude)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (rt.reverseMode) {
            rt.carSpeed -= rt.carAccelStep * deltaTime; // Move backward faster
            if (rt.carSpeed < -konst::kCarMaxReverse) rt.carSpeed = -konst::kCarMaxReverse;
        } else {
            rt.carSpeed += rt.carAccelStep * deltaTime; // Move forward faster
            if (rt.carSpeed > konst::kCarMaxSpeed) rt.carSpeed = konst::kCarMaxSpeed;
        }
    }

    // S = Brake (decrease speed magnitude to 0)
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (rt.carSpeed > 0.0f) {
            rt.carSpeed -= rt.brakeDecelStep * deltaTime;
            if (rt.carSpeed < 0.0f) rt.carSpeed = 0.0f;
        } else if (rt.carSpeed < 0.0f) {
            rt.carSpeed += rt.brakeDecelStep * deltaTime;
            if (rt.carSpeed > 0.0f) rt.carSpeed = 0.0f;
        }
    }

    // Environmental keys (only if extras enabled)
    if (rt.enableExtras) {
        bool shiftHeld = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
        
        // N = adjust daylight
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            if (shiftHeld) {
                rt.daylightLevel += 0.5f * deltaTime;
                if (rt.daylightLevel > 1.0f) rt.daylightLevel = 1.0f;
            } else {
                rt.daylightLevel -= 0.5f * deltaTime;
                if (rt.daylightLevel < 0.0f) rt.daylightLevel = 0.0f;
            }
        }
        
        // M = adjust sun azimuth
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            if (shiftHeld) {
                rt.sunTheta += 0.5f * deltaTime;
            } else {
                rt.sunTheta -= 0.5f * deltaTime;
            }
        }

        // K = adjust sun elevation
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            if (shiftHeld) {
                rt.sunPhi += 0.5f * deltaTime;
            } else {
                rt.sunPhi -= 0.5f * deltaTime;
            }
            if (rt.sunPhi < 0.0f) rt.sunPhi = 0.0f;
            if (rt.sunPhi > konst::kPi / 2.0f) rt.sunPhi = konst::kPi / 2.0f;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        rt.carAngle += rt.carTurnSpeed * std::abs(rt.carSpeed / konst::kCarMaxSpeed) * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        rt.carAngle -= rt.carTurnSpeed * std::abs(rt.carSpeed / konst::kCarMaxSpeed) * deltaTime;
    }
}
