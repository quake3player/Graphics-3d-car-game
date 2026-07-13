#pragma once

namespace konst {
    constexpr int kWindowWidth = 1200;
    constexpr int kWindowHeight = 900;
    
    constexpr float kPi = 3.14159265358979323846f;
    constexpr float kArenaSize = 100.0f; // Bounding area from -50 to 50
    constexpr float kWallHeight = 15.0f;
    
    // Oval track radii
    constexpr float kTrackOuterA = 40.0f;  // outer semi-major (X)
    constexpr float kTrackOuterB = 32.0f;  // outer semi-minor (Z)
    constexpr float kTrackInnerA = 30.0f;  // inner semi-major (X)
    constexpr float kTrackInnerB = 22.0f;  // inner semi-minor (Z)
    constexpr float kTrackMidA = (kTrackOuterA + kTrackInnerA) * 0.5f;
    constexpr float kTrackMidB = (kTrackOuterB + kTrackInnerB) * 0.5f;
    
    constexpr int kNumBuildings = 5;
    constexpr float kCarWidth = 3.8f;
    constexpr float kCarHeight = 2.0f;
    constexpr float kCarLength = 7.0f;
    constexpr float kCarSpawnAngle = 0.0f;
    constexpr float kCarStartSpeed = 0.0f;
    constexpr float kCarMaxSpeed = 40.0f;
    constexpr float kCarMaxReverse = 15.0f;
    constexpr float kNaturalDrag = 2.8f;

    // Gear system — forward speed ranges
    constexpr float kGear1Max = 10.0f;
    constexpr float kGear2Max = 18.0f;
    constexpr float kGear3Max = 24.0f;
    constexpr float kGear4Max = 32.0f;
    constexpr float kGearBreakdownMargin = 8.0f;  // speed above gear max before breakdown
    
    constexpr float kWindmillBaseSpeed = 1.0f;
    constexpr float kWindmillSpeedStep = 1.5f;
    constexpr float kWindmillMaxMult = 8.0f;

    // Camera constants
    constexpr float kFovDegrees = 45.0f;
    constexpr float kNearZ = 0.1f;
    constexpr float kFarZ = 1000.0f;
    constexpr float kSkyCamHeight = 140.0f;
    constexpr float kCarCamHeight = 3.5f;
    constexpr float kCarCamForwardOffset = 0.5f;
    constexpr float kLightCamLookDist = 20.0f;
    constexpr float kCamRotSpeed = 1.5f;
    constexpr float kCamMoveSpeed = 30.0f;
    constexpr float kCamPitchLimit = 1.4f;
    constexpr float kHeliMinHeight = 3.0f;
    constexpr float kHeliMaxHeight = 50.0f;
    constexpr float kHeliMinDist = 5.0f;
    constexpr float kHeliMaxDist = 40.0f;

    // Street lamp
    constexpr int kNumStreetLamps = 8;
    constexpr float kLampHeight = 8.0f;

    // Zoom constants (FOV range in degrees)
    constexpr float kZoomStep = 2.0f;
    constexpr float kMinZoom = 15.0f;
    constexpr float kMaxZoom = 120.0f;

    // Daylight control
    constexpr float kDaylightStep = 0.4f;
    
    // Bullet time factor
    constexpr float kBulletTimeFactor = 0.2f;
    
    // Gimbal light
    constexpr float kGimbalSwingDeg = 30.0f;   // ±30 degrees swing
    constexpr float kGimbalArmLength = 1.8f;    // arm length
    constexpr float kGimbalArmThick = 0.15f;    // arm thickness
    constexpr float kGimbalLanternSize = 0.5f;  // lantern cube size
    
    // Spotlight parameters
    constexpr float kSpotCutoffDeg = 15.0f;
    constexpr float kSpotOuterCutoffDeg = 22.5f;
    constexpr float kSpotLinear = 0.045f;
    constexpr float kSpotQuadratic = 0.0075f;
    
    // Track barrier
    constexpr float kBarrierHeight = 2.0f;
    constexpr float kBarrierThick = 0.6f;
    constexpr int kBarrierSegments = 48;
}
