#include "physics_system.h"
#include <cmath>
#include <algorithm>
#include <array>

// ── OBB helper: compute car's 4 world-space corners ──
static std::array<std::pair<float,float>, 4> getCarCorners(
    float cx, float cz, float angle, float halfW, float halfL)
{
    //  Car forward direction: (-sin(angle), -cos(angle))
    //  Car right direction:   (-cos(angle),  sin(angle))
    float fw_x = -std::sin(angle);
    float fw_z = -std::cos(angle);
    float rt_x = -std::cos(angle);
    float rt_z =  std::sin(angle);

    // Half-extents along each local axis
    float fx = fw_x * halfL, fz = fw_z * halfL;
    float rx = rt_x * halfW, rz = rt_z * halfW;

    return {{
        { cx + fx + rx, cz + fz + rz },   // front-right
        { cx + fx - rx, cz + fz - rz },   // front-left
        { cx - fx - rx, cz - fz - rz },   // rear-left
        { cx - fx + rx, cz - fz + rz },   // rear-right
    }};
}

// ── SAT overlap test: project all corners onto an axis and check overlap ──
static bool overlapOnAxis(
    const std::array<std::pair<float,float>, 4>& cornersA,
    const std::array<std::pair<float,float>, 4>& cornersB,
    float axisX, float axisZ)
{
    float minA =  1e18f, maxA = -1e18f;
    float minB =  1e18f, maxB = -1e18f;
    for (auto& c : cornersA) {
        float p = c.first * axisX + c.second * axisZ;
        minA = std::min(minA, p);
        maxA = std::max(maxA, p);
    }
    for (auto& c : cornersB) {
        float p = c.first * axisX + c.second * axisZ;
        minB = std::min(minB, p);
        maxB = std::max(maxB, p);
    }
    return maxA >= minB && maxB >= minA;
}

// ── OBB vs AABB collision (SAT with 4 axes: 2 from car, 2 world-aligned) ──
static bool obbVsAABB(
    const std::array<std::pair<float,float>, 4>& carCorners,
    float bx, float bz, float bHalfW, float bHalfD,
    float carAngle)
{
    // Building corners (axis-aligned)
    std::array<std::pair<float,float>, 4> bldgCorners = {{
        { bx - bHalfW, bz - bHalfD },
        { bx + bHalfW, bz - bHalfD },
        { bx + bHalfW, bz + bHalfD },
        { bx - bHalfW, bz + bHalfD },
    }};

    // 4 separating axes: 2 world-aligned + 2 car-local
    float axes[4][2] = {
        { 1.0f, 0.0f },                              // world X
        { 0.0f, 1.0f },                              // world Z
        { -std::sin(carAngle), -std::cos(carAngle) }, // car forward
        { -std::cos(carAngle),  std::sin(carAngle) }, // car right
    };

    for (auto& ax : axes) {
        if (!overlapOnAxis(carCorners, bldgCorners, ax[0], ax[1]))
            return false;   // Found a separating axis → no collision
    }
    return true; // All axes overlap → collision
}

// ── Closest distance² from a point to the car OBB (for lamppost checks) ──
static float distSqPointToOBB(
    float px, float pz,
    float cx, float cz, float angle, float halfW, float halfL)
{
    // Transform point into car-local frame
    float dx = px - cx;
    float dz = pz - cz;
    float fw_x = -std::sin(angle), fw_z = -std::cos(angle);
    float rt_x = -std::cos(angle), rt_z =  std::sin(angle);

    float localForward = dx * fw_x + dz * fw_z;
    float localRight   = dx * rt_x + dz * rt_z;

    // Clamp to box extents
    float clampedF = std::max(-halfL, std::min(halfL, localForward));
    float clampedR = std::max(-halfW, std::min(halfW, localRight));

    float diffF = localForward - clampedF;
    float diffR = localRight   - clampedR;
    return diffF * diffF + diffR * diffR;
}

bool checkCollision(const GameRuntime& rt, float nextX, float nextZ) {
    float halfW = konst::kCarWidth  * 0.5f;
    float halfL = konst::kCarLength * 0.5f;
    float halfArena = konst::kArenaSize * 0.5f;

    auto corners = getCarCorners(nextX, nextZ, rt.carAngle, halfW, halfL);

    // ── Wall collision: any corner outside arena ──
    for (auto& c : corners) {
        if (c.first < -halfArena || c.first > halfArena ||
            c.second < -halfArena || c.second > halfArena) {
            return true;
        }
    }

    // ── Building collision: OBB vs AABB via SAT ──
    for (const auto& b : rt.buildings) {
        if (obbVsAABB(corners, b.x, b.z, b.width * 0.5f, b.depth * 0.5f, rt.carAngle))
            return true;
    }

    // ── Lamppost collision: point vs OBB distance ──
    for (const auto& lp : rt.lampPosts) {
        float thresh = lp.radius + 0.3f; // small padding
        if (distSqPointToOBB(lp.x, lp.z, nextX, nextZ, rt.carAngle, halfW, halfL) < thresh * thresh)
            return true;
    }

    // ── Track-side wall collision (unchanged — elliptical distance check) ──
    // Intentionally allowing collision checking for track walls during lighthouse mode
    float angle = std::atan2(nextZ, nextX);
    if (angle < 0) angle += 2.0f * konst::kPi;

    for (const auto& tw : rt.trackWalls) {
        float sa = tw.startAngle;
        float ea = tw.endAngle;
        bool inAngle = false;
        if (sa <= ea) {
            inAngle = (angle >= sa && angle <= ea);
        } else {
            inAngle = (angle >= sa || angle <= ea);
        }
        if (inAngle) {
            float cx = std::cos(angle);
            float sz = std::sin(angle);
            float outR = (konst::kTrackOuterA * konst::kTrackOuterB) /
                         std::sqrt(std::pow(konst::kTrackOuterB * cx, 2) + std::pow(konst::kTrackOuterA * sz, 2));
            float inR = (konst::kTrackInnerA * konst::kTrackInnerB) /
                        std::sqrt(std::pow(konst::kTrackInnerB * cx, 2) + std::pow(konst::kTrackInnerA * sz, 2));
            float dist = std::sqrt(nextX * nextX + nextZ * nextZ);
            float thick = konst::kBarrierThick;
            if (tw.outer) {
                if (std::abs(dist - outR) < (halfW + thick))
                    return true;
            } else {
                if (std::abs(dist - inR) < (halfW + thick))
                    return true;
            }
        }
    }

    return false;
}

void updatePhysics(GameRuntime& rt, float deltaTime) {
    // If car is stopped or broken down, only update windmills, skip movement
    if (rt.carStopped || rt.carBrokenDown) {
        for (auto& b : rt.buildings) {
            b.windmillAngle += konst::kWindmillBaseSpeed * b.windmillSpeedMultiplier * deltaTime;
        }
        return;
    }

    // Gear breakdown check — only when gear system is enabled and moving forward
    if (rt.gearSystemEnabled && rt.carSpeed > 0.0f) {
        float gearMax = konst::kGear1Max;
        switch (rt.currentGear) {
            case 2: gearMax = konst::kGear2Max; break;
            case 3: gearMax = konst::kGear3Max; break;
            case 4: gearMax = konst::kGear4Max; break;
            default: gearMax = konst::kGear1Max; break;
        }
        if (rt.carSpeed > gearMax + konst::kGearBreakdownMargin) {
            rt.carBrokenDown = true;
            rt.carSpeed = 0.0f;
            return;
        }
    }

    // Rolling resistance / drag
    if (rt.carSpeed > 0.0f) {
        rt.carSpeed -= konst::kNaturalDrag * deltaTime;
        if (rt.carSpeed < 0.0f) rt.carSpeed = 0.0f;
    } else if (rt.carSpeed < 0.0f) {
        rt.carSpeed += konst::kNaturalDrag * deltaTime;
        if (rt.carSpeed > 0.0f) rt.carSpeed = 0.0f;
    }

    float nextX = rt.carX - std::sin(rt.carAngle) * rt.carSpeed * deltaTime;
    float nextZ = rt.carZ - std::cos(rt.carAngle) * rt.carSpeed * deltaTime;
    
    if (!checkCollision(rt, nextX, nextZ)) {
        rt.carX = nextX;
        rt.carZ = nextZ;
    } else {
        // HARD STOP per spec: car cannot move after collision until reset
        rt.carSpeed = 0.0f;
        rt.carStopped = true;
    }
    
    // Update windmill rotation
    for (auto& b : rt.buildings) {
        b.windmillAngle += konst::kWindmillBaseSpeed * b.windmillSpeedMultiplier * deltaTime;
    }
    
    // Update spotlight swing phase (the position/direction calculation is done in main.cpp)
    for (auto& s : rt.spotlights) {
        s.swingPhase += s.swingSpeed * deltaTime;
    }

    // Evaluate Lighthouse Parking Challenge
    if (rt.lighthouseMode && !rt.challengeSuccess) {
        float dx = rt.carX - 45.0f;
        float dz = rt.carZ - 45.0f;
        // Relaxed condition: distance < 4.0 units and almost stopped
        if (dx * dx + dz * dz < 16.0f && std::abs(rt.carSpeed) < 2.0f) {
            rt.challengeSuccess = true;
            rt.carSpeed = 0.0f;
            rt.carStopped = true;
        }
    }
}
