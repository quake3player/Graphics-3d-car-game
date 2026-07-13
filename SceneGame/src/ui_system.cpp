#include "ui_system.h"
#include <cstdio>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cmath>
#include "game_logic.h"

void initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
}

void renderUI(GameRuntime& rt, float fps, bool& bulletTime) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ═══════════════════════════════════════════════════
    //  WINDOW 1: Vehicle Status (top-left)
    // ═══════════════════════════════════════════════════
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360, 340), ImGuiCond_FirstUseEver);
    ImGui::Begin("Vehicle Status");
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Separator();

    // Camera
    const char* cameras[] = { "Sky View", "Car View", "Ground View", "Light View", "Helicopter" };
    ImGui::Combo("View Mode", &rt.cameraIndex, cameras, 5);
    switch (rt.cameraIndex) {
        case 0: ImGui::Text("Offset: (%.1f, %.1f)", rt.skyOffsetX, rt.skyOffsetZ); break;
        case 1: ImGui::Text("Yaw: %.1f  Pitch: %.1f", rt.carCamYaw * 57.3f, rt.carCamPitch * 57.3f); break;
        case 2:
            ImGui::Text("Pos: (%.1f, %.1f, %.1f)", rt.groundCamX, rt.groundCamY, rt.groundCamZ);
            ImGui::SliderAngle("Yaw", &rt.groundCamYaw);
            ImGui::SliderAngle("Pitch", &rt.groundCamPitch, -89.0f, 89.0f);
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Yellow cube marks camera");
            break;
        case 3: ImGui::Text("Tracking Light: %d", rt.selectedLight + 1); break;
        case 4: ImGui::Text("Height: %.1f  Dist: %.1f", rt.heliHeight, rt.heliDist); break;
    }
    ImGui::Separator();

    // Speed + status
    ImGui::Text("Vehicle Speed:");
    ImGui::ProgressBar(std::abs(rt.carSpeed) / konst::kCarMaxSpeed, ImVec2(-1.0f, 0.0f));
    ImGui::Text("Position: (%.1f, %.1f)  Heading: %.1f deg", rt.carX, rt.carZ, rt.carAngle * 57.2958f);
    ImGui::Text("Mode: %s", rt.reverseMode ? "REVERSE" : "FORWARD");
    if (rt.carStopped && !rt.challengeSuccess) {
        ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "COLLISION! Press X to reset.");
    }
    if (rt.carBrokenDown) {
        ImGui::TextColored(ImVec4(1, 0.5f, 0.0f, 1), "ENGINE BREAKDOWN! Press X to reset.");
    }

    // Car Model
    if (!rt.carModelNames.empty()) {
        ImGui::Separator();
        if (ImGui::BeginCombo("Car Model", rt.carModelNames[rt.selectedCarModel].c_str())) {
            for (int i = 0; i < (int)rt.carModelNames.size(); i++) {
                bool isSelected = (rt.selectedCarModel == i);
                if (ImGui::Selectable(rt.carModelNames[i].c_str(), isSelected))
                    rt.selectedCarModel = i;
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    ImGui::End();

    // ═══════════════════════════════════════════════════
    //  WINDOW 2: Tuning & Parameters (below status)
    // ═══════════════════════════════════════════════════
    ImGui::SetNextWindowPos(ImVec2(10, 360), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360, 420), ImGuiCond_FirstUseEver);
    ImGui::Begin("Tuning & Parameters");

    ImGui::Text("Sensitivity");
    ImGui::SliderFloat("Acceleration", &rt.carAccelStep, 1.0f, 20.0f);
    ImGui::SliderFloat("Brake", &rt.brakeDecelStep, 10.0f, 60.0f);
    ImGui::SliderFloat("Turn", &rt.carTurnSpeed, 0.1f, 3.0f);
    ImGui::Separator();

    ImGui::Checkbox("Enable Gear System", &rt.gearSystemEnabled);
    if (rt.gearSystemEnabled) {
        ImGui::Text("Gear: %d / 4", rt.currentGear);
        float gearMin = 0.0f, gearMax = 10.0f;
        switch (rt.currentGear) {
            case 1: gearMin = 0.0f;  gearMax = 10.0f; break;
            case 2: gearMin = 10.0f; gearMax = 18.0f; break;
            case 3: gearMin = 18.0f; gearMax = 24.0f; break;
            case 4: gearMin = 24.0f; gearMax = 32.0f; break;
        }
        ImGui::Text("Range: %.0f - %.0f m/s (exceed +8 = breakdown)", gearMin, gearMax);
    }
    ImGui::Separator();

    ImGui::Text("Windmill Speed");
    for (size_t i = 0; i < rt.buildings.size(); i++) {
        char label[64];
        snprintf(label, sizeof(label), "Building %zu", i + 1);
        ImGui::SliderFloat(label, &rt.buildings[i].windmillSpeedMultiplier, 0.0f, konst::kWindmillMaxMult, "%.2f x");
    }
    ImGui::Separator();

    ImGui::SliderFloat("Daylight", &rt.daylightLevel, 0.0f, 1.0f, "%.2f");
    ImGui::Checkbox("Bullet Time", &bulletTime);

    if (rt.enableExtras) {
        ImGui::Separator();
        ImGui::Text("Challenge Modes");
        if (ImGui::Checkbox("Lighthouse Parking", &rt.lighthouseMode)) {
            resetWorld(rt);
        }
        if (rt.lighthouseMode) {
            ImGui::Checkbox("Lighthouse Light", &rt.lighthouseLightEnabled);
            if (rt.challengeSuccess)
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "SUCCESS! PERFECT PARKING!");
            else
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Park at corner (45, 45)!");
        }
    }
    ImGui::End();

    // ═══════════════════════════════════════════════════
    //  WINDOW 3: Controls Reference (right side)
    // ═══════════════════════════════════════════════════
    ImGui::SetNextWindowPos(ImVec2(380, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 340), ImGuiCond_FirstUseEver);
    ImGui::Begin("Keyboard Controls");

    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1), "Driving");
    ImGui::BulletText("W: Accelerate  S: Brake");
    ImGui::BulletText("A: Steer Left  D: Steer Right");
    ImGui::BulletText("R: Toggle Reverse");
    ImGui::BulletText("G: Gear Up   H: Gear Down");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1), "Camera");
    ImGui::BulletText("Arrow Keys: Camera Adjust");
    ImGui::BulletText("C: Cycle Camera Mode");
    ImGui::BulletText("1/2/3/4: Select Light");
    ImGui::BulletText("+/-: Zoom In/Out");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1), "Environment");
    ImGui::BulletText("P/O: Windmill Faster/Slower");
    ImGui::BulletText("[/]: Darker/Brighter");
    ImGui::BulletText("B: Bullet Time");
    ImGui::BulletText("X: Reset   Esc: Exit");

    if (rt.enableExtras) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "N/Shift+N: Daylight");
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "M/Shift+M: Sun Azimuth");
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "K/Shift+K: Sun Elevation");
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
