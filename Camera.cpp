#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }


    // Return the view matrix using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    // Update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        speed = speed / 2;
        switch (direction) {
        case MOVE_FORWARD:  // Move forward along the camera's front direction
            cameraPosition += cameraFrontDirection * speed;
            break;
        case MOVE_BACKWARD: // Move backward along the camera's front direction
            cameraPosition -= cameraFrontDirection * speed;
            break;
        case MOVE_RIGHT:    // Move right along the camera's right direction
            cameraPosition += cameraRightDirection * speed;
            break;
        case MOVE_LEFT:     // Move left along the camera's right direction
            cameraPosition -= cameraRightDirection * speed;
            break;
        case MOVE_UP:       // Move up along the camera's up direction (spacebar)
            cameraPosition += cameraUpDirection * speed;
            break;
        case MOVE_DOWN:     // Move down along the camera's up direction (c key)
            cameraPosition -= cameraUpDirection * speed;
            break;
        }
    }

    // Update the camera internal parameters following a camera rotate event
    // yaw - camera rotation around the y axis
    // pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 newFront;

        // Update pitch and yaw to change the camera direction
        newFront.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

        cameraFrontDirection = glm::normalize(newFront);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }
    void Camera::setPosition(const glm::vec3& position) { cameraPosition = position; }
    void Camera::setTarget(const glm::vec3& target) {
        cameraTarget = target;
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
    }

    glm::vec3 Camera::getCameraFront() {
        return cameraFrontDirection;
    }

    glm::vec3 Camera::getCameraRight() {
        return cameraRightDirection;
    }

}
