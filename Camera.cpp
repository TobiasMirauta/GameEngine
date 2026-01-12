#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        yaw = glm::degrees(std::asin(glm::clamp(cameraFrontDirection.y, -1.0f, 1.0f)));
        pitch = glm::degrees(std::atan2(cameraFrontDirection.z, cameraFrontDirection.x));
        //TODO - Update the rest of camera parameters

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }

    void Camera::MouseLook(double mouseX, double mouseY) {
        
        bool& firstLook = this->mouseFirstLook;

        glm::vec3 frontDirection = glm::normalize(cameraTarget - cameraPosition);
        glm::vec3 rightDirection = glm::normalize(glm::cross(frontDirection, cameraUpDirection));
        glm::vec3 upDirection = glm::normalize(cameraUpDirection);

        pitch = glm::degrees(asin(glm::clamp(frontDirection.y, -1.0f, 1.0f)));
        yaw = glm::degrees(atan2(frontDirection.z, frontDirection.x));

        glm::vec2 crtMousePosition = glm::vec2(mouseX, mouseY);
        if (firstLook) {
            oldMousePosition = crtMousePosition;
            firstLook = false;
            return;
        }
        glm::vec2 mouseDelta = crtMousePosition - oldMousePosition;
        oldMousePosition = crtMousePosition;

        const float intensitate = 0.12f;
        yaw += mouseDelta.x * intensitate;
        pitch += -mouseDelta.y * intensitate;
        
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(front);

        rightDirection = glm::normalize(glm::cross(rightDirection, worldUp));
        upDirection = glm::normalize(glm::cross(rightDirection, frontDirection));
        
        cameraTarget = cameraPosition + cameraFrontDirection;
        fprintf(stdout, "front: %f %f %f\n", frontDirection.x, frontDirection.y, frontDirection.z);

        }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
       // glm::vec3 front = glm::normalize(cameraTarget - cameraPosition);
       // glm::vec3 right = glm::normalize(glm::cross(front, cameraUpDirection));
        glm::vec3 up = glm::normalize(cameraUpDirection);
        glm::vec3 front = glm::normalize(glm::vec3(cameraFrontDirection.x, 0.0f, cameraFrontDirection.z));
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));


        if (direction == MOVE_FORWARD) {
            cameraPosition += front * speed;
            cameraTarget += front * speed;
        }
        else if (direction == MOVE_BACKWARD) {
            cameraPosition -= front * speed;
            cameraTarget -= front * speed;
        }
        else if (direction == MOVE_RIGHT) {
            cameraPosition += right * speed;
            cameraTarget += right * speed;
        }
        else if (direction == MOVE_LEFT) {
            cameraPosition -= right * speed;
            cameraTarget -= right * speed;
        }
      /* else if (direction == MOVE_UP) {
            cameraPosition += up * speed;
            cameraTarget += up * speed;

        }
        else if (direction == MOVE_DOWN) {
            cameraPosition -= up * speed;
            cameraTarget -= up * speed;
        }*/
        cameraPosition.y = 16.0f;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float auxPitch, float auxYaw) {
        yaw += auxYaw;
        pitch += auxPitch;

        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        cameraTarget = cameraPosition + front;
        cameraFrontDirection = front;
        cameraRightDirection = glm::normalize(glm::cross(front, cameraUpDirection));

        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));

    }
}