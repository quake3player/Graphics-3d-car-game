#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include "constants.h"

struct SpotlightState {
    float x, y, z;          // current position (updated each frame)
    float dirX, dirY, dirZ;  // current direction (updated each frame)
    float r, g, b;           // light color
    float swingPhase;        // current swing angle (radians)
    float swingSpeed;        // how fast the light swings
};

struct Building {
    float x, z;
    float width, depth, height;
    float r, g, b;
    float windmillAngle; 
    float windmillSpeedMultiplier; 
    int stories;      // 1, 2, or 3
    int textureType;   // 0 = brick, 1 = wood
};

struct TrackWall {
    float startAngle; // parametric angle start (radians)
    float endAngle;   // parametric angle end (radians)
    bool outer;       // true = outer edge, false = inner edge
};

struct LampPost {
    float x, z;       // position on the ground
    float radius;     // collision radius
};

struct GameRuntime {
    GLuint shaderProgram;
    GLuint texBrick, texWood, texRoad;
    GLuint texConcrete, texStone;
    
    float lastTime;
    
    float carX, carY, carZ;
    float carAngle; 
    float carSpeed;
    bool  carStopped;  // true after collision — no movement until reset
    
    // Driving state
    bool reverseMode = false;    // toggled by R
    int currentGear = 1;         // 1–4
    bool gearSystemEnabled = false; // checkbox toggle
    bool carBrokenDown = false;  // gear violation breakdown
    
    int cameraIndex; 
    float groundCamYaw, groundCamPitch;
    float groundCamX, groundCamY, groundCamZ;
    
    // Sky view state
    float skyOffsetX = 0.0f;
    float skyOffsetZ = 0.0f;
    
    // Car view state (yaw/pitch offset from forward)
    float carCamYaw = 0.0f;
    float carCamPitch = 0.0f;
    
    // Light view state
    int selectedLight = 0; // 0–3
    
    // Helicopter view state
    float heliHeight = 10.0f;
    float heliDist = 12.0f;
    
    std::vector<Building> buildings;
    std::vector<SpotlightState> spotlights;
    std::vector<TrackWall> trackWalls;
    std::vector<LampPost> lampPosts;
    
    float zoomFov = 45.0f;      // current FOV in degrees (zoom)
    
    // Environment
    bool enableExtras = false;
    bool enableShadows = false;
    bool dayNightToggle = true; // true = Day, false = Night
    float daylightLevel = 0.7f;  // controlled by toggle mostly, but kept for smooth transition if needed.
    float sunTheta = 0.3f;       // Sun Azimuth
    float sunPhi = 0.5f;         // Sun Elevation
    
    // Sensitivity
    float carAccelStep = 6.0f;
    float carDecelStep = 16.0f;
    float brakeDecelStep = 30.0f;
    float carTurnSpeed = 2.0f;
    
    // Challenge Mode
    bool lighthouseMode = false;
    bool lighthouseLightEnabled = true;
    bool challengeSuccess = false;
    
    // Car model selection (0 = original hardcoded, 1+ = OBJ models)
    int selectedCarModel = 0;
    std::vector<std::string> carModelNames;
};
