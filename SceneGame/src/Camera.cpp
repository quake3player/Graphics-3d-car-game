#include "Camera.h"
#include "constants.h"
#include <cmath>

glm::mat4 CameraManager::getViewMatrix(
    const glm::vec3& carPos, float carAngle,
    const glm::vec3& lightPos, const glm::vec3& lightDir) const 
{
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 carForward{-std::sin(carAngle), 0.0f, -std::cos(carAngle)};
    
    if (mode == SKY) {
        glm::vec3 eye(skyOffsetX, konst::kSkyCamHeight, skyOffsetZ);
        glm::vec3 target(skyOffsetX, 0.0f, skyOffsetZ);
        return glm::lookAt(eye, target, glm::vec3(0, 0, -1));
    } 
    else if (mode == CAR) {
        glm::vec3 pos = carPos + glm::vec3(0, konst::kCarCamHeight, 0)
                      + carForward * konst::kCarCamForwardOffset;
        float totalYaw = carAngle + carCamYaw;
        float pitch = carCamPitch;
        glm::vec3 dir{
            -std::sin(totalYaw) * std::cos(pitch),
            std::sin(pitch),
            -std::cos(totalYaw) * std::cos(pitch)
        };
        return glm::lookAt(pos, pos + dir, up);
    } 
    else if (mode == GROUND) {
        // Camera is above the marker cube so marker doesn't block the view
        glm::vec3 camPos = groundPos + glm::vec3(0, 2.0f, 0);
        glm::vec3 dir{
            std::sin(groundYaw) * std::cos(groundPitch),
            std::sin(groundPitch),
            std::cos(groundYaw) * std::cos(groundPitch)
        };
        return glm::lookAt(camPos, camPos + dir, up);
    } 
    else if (mode == LIGHT) {
        // Camera positioned BEHIND the light, looking along the light direction
        // Offset back from light position so we can see the road, not just the bulb
        glm::vec3 offset = glm::normalize(lightDir) * 3.0f; // move 3 units along light dir
        glm::vec3 camPos = lightPos - offset + glm::vec3(0, 1.5f, 0); // step back + slightly above
        glm::vec3 target = lightPos + lightDir * konst::kLightCamLookDist;
        return glm::lookAt(camPos, target, up);
    } 
    else if (mode == HELICOPTER) {
        glm::vec3 pos = carPos - carForward * heliDist + glm::vec3(0, heliHeight, 0);
        return glm::lookAt(pos, carPos, up);
    }
    
    return glm::mat4(1.0f);
}

glm::mat4 CameraManager::getProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(zoomFov), aspect, konst::kNearZ, konst::kFarZ);
}
