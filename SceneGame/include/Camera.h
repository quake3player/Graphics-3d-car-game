#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraManager {
public:
    enum ViewMode { SKY, CAR, GROUND, LIGHT, HELICOPTER };

    ViewMode mode = SKY;
    
    // Sky view panning
    float skyOffsetX = 0.0f;
    float skyOffsetZ = 0.0f;
    
    // Ground view state
    glm::vec3 groundPos{0.0f, 5.0f, 45.0f};
    float groundYaw = 0.0f;
    float groundPitch = 0.0f;
    
    // Car cam look offsets (yaw/pitch from forward)
    float carCamYaw = 0.0f;
    float carCamPitch = 0.0f;
    
    // Light view
    int selectedLight = 0;
    
    // Helicopter
    float heliHeight = 10.0f;
    float heliDist = 12.0f;
    
    float zoomFov = 45.0f;  // dynamic FOV for zoom

    glm::mat4 getViewMatrix(
        const glm::vec3& carPos, float carAngle,
        const glm::vec3& lightPos, const glm::vec3& lightDir
    ) const;
    
    glm::mat4 getProjectionMatrix(float aspect) const;
};
