#include "game_logic.h"
#include <cmath>
#include <iterator>

void resetWorld(GameRuntime& rt) {
    // Place car on the left side of the oval (x < 0) no matter the mode
    float spawnAngle = konst::kPi; // Left side
    rt.carX = konst::kTrackMidA * std::cos(spawnAngle);
    rt.carY = (konst::kCarHeight * 0.5f) + 0.35f; // Raised to prevent wheel clipping
    rt.carZ = konst::kTrackMidB * std::sin(spawnAngle);
    rt.carAngle = 0.0f;
    rt.challengeSuccess = false;
    rt.carSpeed = konst::kCarStartSpeed;
    rt.carStopped = false;
    
    // Reset driving state
    rt.reverseMode = false;
    rt.currentGear = 1;
    rt.carBrokenDown = false;
    
    rt.cameraIndex = 0; 
    rt.groundCamYaw = -konst::kPi * 0.75f;
    rt.groundCamPitch = -0.1f;
    rt.skyOffsetX = 0.0f;
    rt.skyOffsetZ = 0.0f;
    rt.carCamYaw = 0.0f;
    rt.carCamPitch = 0.0f;
    rt.selectedLight = 0;
    rt.heliHeight = 10.0f;
    rt.heliDist = 12.0f;
    
    rt.buildings.clear();
    rt.spotlights.clear();
    rt.trackWalls.clear();
    rt.lampPosts.clear();
    
    // 4 buildings with different textures: 0=brick, 1=wood, 2=concrete, 3=stone
    Building presets[] = {
        // x, z, width, depth, height, r, g, b, windmillAngle, windmillSpeedMult, stories, textureType
        { 0.0f,  44.0f,  10.0f,  8.0f,  8.0f, 0.85f, 0.55f, 0.40f, 0.0f, 1.00f, 1, 0},  // brick, 1-story
        { 42.0f,  0.0f,   9.0f, 10.0f, 14.0f, 0.55f, 0.42f, 0.30f, 0.0f, 1.50f, 2, 1},  // wood, 2-story
        {-42.0f,  0.0f,   8.0f,  9.0f, 18.0f, 0.70f, 0.70f, 0.72f, 0.0f, 0.90f, 3, 2},  // concrete, 3-story
        { 0.0f, -44.0f, 11.0f,  8.0f, 24.0f, 0.72f, 0.65f, 0.55f, 0.0f, 1.20f, 4, 3},  // stone, 4-story
    };

    SpotlightState spotlightPresets[] = {
        { 0.0f,   8.5f,  44.0f,   0,0,0,   1.0f, 0.3f, 0.3f,  0.0f, 0.9f},
        { 42.0f, 14.5f,   0.0f,   0,0,0,   0.3f, 0.8f, 1.0f,  1.0f, 1.2f},
        {-42.0f, 18.5f,   0.0f,   0,0,0,   0.3f, 1.0f, 0.4f,  2.0f, 1.0f},
        { 0.0f,  24.5f, -44.0f,   0,0,0,   1.0f, 1.0f, 0.3f,  3.0f, 1.1f},
    };

    rt.buildings.assign(std::begin(presets), std::end(presets));
    rt.spotlights.assign(std::begin(spotlightPresets), std::end(spotlightPresets));

    // Lighthouse logic
    if (rt.lighthouseMode) {
        SpotlightState lighthouseLight = { 38.0f, 26.5f, 38.0f, 0,0,0, 1.0f, 1.0f, 0.2f, 0.0f, 0.5f }; // Yellow light
        rt.spotlights.push_back(lighthouseLight);
    }
    
    // Track-side barrier walls
    rt.trackWalls = {
        { 0.2f,  1.2f,  true },
        { 3.5f,  4.5f,  false },
        { 2.0f,  2.8f,  true },
    };

    // Initialize lampposts on the outer edge of the road
    for (int i = 0; i < konst::kNumStreetLamps; i++) {
        float angle = i * 2.0f * konst::kPi / konst::kNumStreetLamps;
        LampPost lp;
        lp.x = std::cos(angle) * (konst::kTrackOuterA + 1.0f);
        lp.z = std::sin(angle) * (konst::kTrackOuterB + 1.0f);
        lp.radius = 0.5f;
        rt.lampPosts.push_back(lp);
    }
    
    // Ground cam: in front of building 1 (top building at z=44), looking at road
    rt.groundCamX = 5.0f;
    rt.groundCamY = 6.0f;
    rt.groundCamZ = 38.0f;   // just inside of building edge
    rt.groundCamYaw = konst::kPi;  // face toward -Z (toward road/center)
    rt.groundCamPitch = -0.2f;     // slight downward look
}

void initializeGameRuntime(GameRuntime& rt) {
    resetWorld(rt);
}
