#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <string>

namespace gps {

    enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN };

    class Camera
    {
    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        //return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        glm::vec3 getCameraPosition();
        //update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction, float speed);
        //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        float yaw;
        //pitch - camera rotation around the x axis
        float pitch;
        void rotate(float pitch, float yaw);
        void MouseLook(double mouseX, double mouseY);
        bool mouseFirstLook;

    private:
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
        glm::vec2 oldMousePosition;
    };

}

#endif /* Camera_hpp */
